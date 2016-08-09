// Copyright (c) 2009-2011, Tor M. Aamodt, Inderpreet Singh, Timothy Rogers,
// Ahmed ElTantawy
// The University of British Columbia
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
// Neither the name of The University of British Columbia nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



#include "abstract_hardware_model.h"
#include "cuda-sim/memory.h"
#include "cuda-sim/ptx_ir.h"
#include "cuda-sim/ptx-stats.h"
#include "cuda-sim/cuda-sim.h"
#include "gpgpu-sim/gpu-sim.h"
#include "option_parser.h"
#include <algorithm>

unsigned mem_access_t::sm_next_access_uid = 0;   
unsigned warp_inst_t::sm_next_uid = 0;

void move_warp( warp_inst_t *&dst, warp_inst_t *&src )
{
   assert( dst->empty() );
   warp_inst_t* temp = dst;
   dst = src;
   src = temp;
   src->clear();
}


void gpgpu_functional_sim_config::reg_options(class OptionParser * opp)
{
	option_parser_register(opp, "-gpgpu_ptx_use_cuobjdump", OPT_BOOL,
                 &m_ptx_use_cuobjdump,
                 "Use cuobjdump to extract ptx and sass from binaries",
#if (CUDART_VERSION >= 4000)
                 "1"
#else
                 "0"
#endif
                 );
	option_parser_register(opp, "-gpgpu_experimental_lib_support", OPT_BOOL,
	                 &m_experimental_lib_support,
	                 "Try to extract code from cuda libraries [Broken because of unknown cudaGetExportTable]",
	                 "0");
    option_parser_register(opp, "-gpgpu_ptx_convert_to_ptxplus", OPT_BOOL,
                 &m_ptx_convert_to_ptxplus,
                 "Convert SASS (native ISA) to ptxplus and run ptxplus",
                 "0");
    option_parser_register(opp, "-gpgpu_ptx_force_max_capability", OPT_UINT32,
                 &m_ptx_force_max_capability,
                 "Force maximum compute capability",
                 "0");
   option_parser_register(opp, "-gpgpu_ptx_inst_debug_to_file", OPT_BOOL, 
                &g_ptx_inst_debug_to_file, 
                "Dump executed instructions' debug information to file", 
                "0");
   option_parser_register(opp, "-gpgpu_ptx_inst_debug_file", OPT_CSTR, &g_ptx_inst_debug_file, 
                  "Executed instructions' debug output file",
                  "inst_debug.txt");
   option_parser_register(opp, "-gpgpu_ptx_inst_debug_thread_uid", OPT_INT32, &g_ptx_inst_debug_thread_uid, 
               "Thread UID for executed instructions' debug output", 
               "1");
}

void gpgpu_functional_sim_config::ptx_set_tex_cache_linesize(unsigned linesize)
{
   m_texcache_linesize = linesize;
}

gpgpu_t::gpgpu_t( const gpgpu_functional_sim_config &config )
    : m_function_model_config(config)
{
   m_global_mem = new memory_space_impl<8192>("global",64*1024);
   m_tex_mem = new memory_space_impl<8192>("tex",64*1024);
   m_surf_mem = new memory_space_impl<8192>("surf",64*1024);

   m_dev_malloc=GLOBAL_HEAP_START; 

   if(m_function_model_config.get_ptx_inst_debug_to_file() != 0) 
      ptx_inst_debug_file = fopen(m_function_model_config.get_ptx_inst_debug_file(), "w");
}

address_type line_size_based_tag_func(new_addr_type address, new_addr_type line_size)
{
   //gives the tag for an address based on a given line size
   return address & ~(line_size-1);
}

const char * mem_access_type_str(enum mem_access_type access_type)
{
   #define MA_TUP_BEGIN(X) static const char* access_type_str[] = {
   #define MA_TUP(X) #X
   #define MA_TUP_END(X) };
   MEM_ACCESS_TYPE_TUP_DEF
   #undef MA_TUP_BEGIN
   #undef MA_TUP
   #undef MA_TUP_END

   assert(access_type < NUM_MEM_ACCESS_TYPE); 

   return access_type_str[access_type]; 
}


void warp_inst_t::clear_active( const active_mask_t &inactive ) {
    active_mask_t test = m_warp_active_mask;
    test &= inactive;
    assert( test == inactive ); // verify threads being disabled were active
    m_warp_active_mask &= ~inactive;
}

void warp_inst_t::set_not_active( unsigned lane_id ) {
    m_warp_active_mask.reset(lane_id);
}

void warp_inst_t::set_active( unsigned lane_id ) {
    m_warp_active_mask.set(lane_id);
}


void warp_inst_t::set_active( const active_mask_t &active ) {
   m_warp_active_mask = active;
   if( m_isatomic ) {
      for( unsigned i=0; i < m_config->warp_size; i++ ) {
         if( !m_warp_active_mask.test(i) ) {
             m_per_scalar_thread[i].callback.function = NULL;
             m_per_scalar_thread[i].callback.instruction = NULL;
             m_per_scalar_thread[i].callback.thread = NULL;
         }
      }
   }
}

void warp_inst_t::do_atomic(bool forceDo) {
    do_atomic( m_warp_active_mask,forceDo );
}


void warp_inst_t::do_atomic( const active_mask_t& access_mask,bool forceDo ) {
    assert( m_isatomic && (!m_empty||forceDo) );
    for( unsigned i=0; i < m_config->warp_size; i++ )
    {
        if( access_mask.test(i) )
        {
            dram_callback_t &cb = m_per_scalar_thread[i].callback;
            if( cb.thread )
                cb.function(cb.instruction, cb.thread);
        }
    }
}

void warp_inst_t::broadcast_barrier_reduction(const active_mask_t& access_mask)
{
	for( unsigned i=0; i < m_config->warp_size; i++ )
    {
        if( access_mask.test(i) )
        {
            dram_callback_t &cb = m_per_scalar_thread[i].callback;
            if( cb.thread ){
                cb.function(cb.instruction, cb.thread);
            }
        }
    }
}

/* Used for the virtualization of AWARE SIMT tables */
void warp_inst_t::inject_mem_acccesses(mem_access_t acc)
{
	m_accessq.push_back(acc);
}


void warp_inst_t::generate_mem_accesses()
{
    if( empty() || op == MEMORY_BARRIER_OP || m_mem_accesses_created ) 
        return;
    if ( !((op == LOAD_OP) || (op == STORE_OP)) )
        return; 
    if( m_warp_active_mask.count() == 0 ) 
        return; // predicated off

    const size_t starting_queue_size = m_accessq.size();

    assert( is_load() || is_store() );
    assert( m_per_scalar_thread_valid ); // need address information per thread

    bool is_write = is_store();

    mem_access_type access_type;
    switch (space.get_type()) {
    case const_space:
    case param_space_kernel: 
        access_type = CONST_ACC_R; 
        break;
    case tex_space: 
        access_type = TEXTURE_ACC_R;   
        break;
    case global_space:       
        access_type = is_write? GLOBAL_ACC_W: GLOBAL_ACC_R;   
        break;
    case local_space:
    case param_space_local:  
        access_type = is_write? LOCAL_ACC_W: LOCAL_ACC_R;   
        break;
    case shared_space: break;
    default: assert(0); break; 
    }

    // Calculate memory accesses generated by this warp
    new_addr_type cache_block_size = 0; // in bytes 

    switch( space.get_type() ) {
    case shared_space: {
        unsigned subwarp_size = m_config->warp_size / m_config->mem_warp_parts;
        unsigned total_accesses=0;
        for( unsigned subwarp=0; subwarp <  m_config->mem_warp_parts; subwarp++ ) {

            // data structures used per part warp 
            std::map<unsigned,std::map<new_addr_type,unsigned> > bank_accs; // bank -> word address -> access count

            // step 1: compute accesses to words in banks
            for( unsigned thread=subwarp*subwarp_size; thread < (subwarp+1)*subwarp_size; thread++ ) {
                if( !active(thread) ) 
                    continue;
                new_addr_type addr = m_per_scalar_thread[thread].memreqaddr[0];
                //FIXME: deferred allocation of shared memory should not accumulate across kernel launches
                //assert( addr < m_config->gpgpu_shmem_size ); 
                unsigned bank = m_config->shmem_bank_func(addr);
                new_addr_type word = line_size_based_tag_func(addr,m_config->WORD_SIZE);
                bank_accs[bank][word]++;
            }

            if (m_config->shmem_limited_broadcast) {
                // step 2: look for and select a broadcast bank/word if one occurs
                bool broadcast_detected = false;
                new_addr_type broadcast_word=(new_addr_type)-1;
                unsigned broadcast_bank=(unsigned)-1;
                std::map<unsigned,std::map<new_addr_type,unsigned> >::iterator b;
                for( b=bank_accs.begin(); b != bank_accs.end(); b++ ) {
                    unsigned bank = b->first;
                    std::map<new_addr_type,unsigned> &access_set = b->second;
                    std::map<new_addr_type,unsigned>::iterator w;
                    for( w=access_set.begin(); w != access_set.end(); ++w ) {
                        if( w->second > 1 ) {
                            // found a broadcast
                            broadcast_detected=true;
                            broadcast_bank=bank;
                            broadcast_word=w->first;
                            break;
                        }
                    }
                    if( broadcast_detected ) 
                        break;
                }
            
                // step 3: figure out max bank accesses performed, taking account of broadcast case
                unsigned max_bank_accesses=0;
                for( b=bank_accs.begin(); b != bank_accs.end(); b++ ) {
                    unsigned bank_accesses=0;
                    std::map<new_addr_type,unsigned> &access_set = b->second;
                    std::map<new_addr_type,unsigned>::iterator w;
                    for( w=access_set.begin(); w != access_set.end(); ++w ) 
                        bank_accesses += w->second;
                    if( broadcast_detected && broadcast_bank == b->first ) {
                        for( w=access_set.begin(); w != access_set.end(); ++w ) {
                            if( w->first == broadcast_word ) {
                                unsigned n = w->second;
                                assert(n > 1); // or this wasn't a broadcast
                                assert(bank_accesses >= (n-1));
                                bank_accesses -= (n-1);
                                break;
                            }
                        }
                    }
                    if( bank_accesses > max_bank_accesses ) 
                        max_bank_accesses = bank_accesses;
                }

                // step 4: accumulate
                total_accesses+= max_bank_accesses;
            } else {
                // step 2: look for the bank with the maximum number of access to different words 
                unsigned max_bank_accesses=0;
                std::map<unsigned,std::map<new_addr_type,unsigned> >::iterator b;
                for( b=bank_accs.begin(); b != bank_accs.end(); b++ ) {
                    max_bank_accesses = std::max(max_bank_accesses, (unsigned)b->second.size());
                }

                // step 3: accumulate
                total_accesses+= max_bank_accesses;
            }
        }
        assert( total_accesses > 0 && total_accesses <= m_config->warp_size );
        cycles = total_accesses; // shared memory conflicts modeled as larger initiation interval 
        ptx_file_line_stats_add_smem_bank_conflict( pc, total_accesses );
        break;
    }

    case tex_space: 
        cache_block_size = m_config->gpgpu_cache_texl1_linesize;
        break;
    case const_space:  case param_space_kernel:
        cache_block_size = m_config->gpgpu_cache_constl1_linesize; 
        break;

    case global_space: case local_space: case param_space_local:
        if( m_config->gpgpu_coalesce_arch == 13 ) {
           if(isatomic())
               memory_coalescing_arch_13_atomic(is_write, access_type);
           else
               memory_coalescing_arch_13(is_write, access_type);
        } else abort();

        break;

    default:
        abort();
    }

    if( cache_block_size ) {
        assert( m_accessq.empty() );
        mem_access_byte_mask_t byte_mask; 
        std::map<new_addr_type,active_mask_t> accesses; // block address -> set of thread offsets in warp
        std::map<new_addr_type,active_mask_t>::iterator a;
        for( unsigned thread=0; thread < m_config->warp_size; thread++ ) {
            if( !active(thread) ) 
                continue;
            new_addr_type addr = m_per_scalar_thread[thread].memreqaddr[0];
            unsigned block_address = line_size_based_tag_func(addr,cache_block_size);
            accesses[block_address].set(thread);
            unsigned idx = addr-block_address; 
            for( unsigned i=0; i < data_size; i++ ) 
                byte_mask.set(idx+i);
        }
        for( a=accesses.begin(); a != accesses.end(); ++a ) 
            m_accessq.push_back( mem_access_t(access_type,a->first,cache_block_size,is_write,a->second,byte_mask) );
    }

    if ( space.get_type() == global_space ) {
        ptx_file_line_stats_add_uncoalesced_gmem( pc, m_accessq.size() - starting_queue_size );
    }
    m_mem_accesses_created=true;
}

void warp_inst_t::memory_coalescing_arch_13( bool is_write, mem_access_type access_type )
{
    // see the CUDA manual where it discusses coalescing rules before reading this
    unsigned segment_size = 0;
    unsigned warp_parts = m_config->mem_warp_parts;
    switch( data_size ) {
    case 1: segment_size = 32; break;
    case 2: segment_size = 64; break;
    case 4: case 8: case 16: segment_size = 128; break;
    }
    unsigned subwarp_size = m_config->warp_size / warp_parts;

    for( unsigned subwarp=0; subwarp <  warp_parts; subwarp++ ) {
        std::map<new_addr_type,transaction_info> subwarp_transactions;

        // step 1: find all transactions generated by this subwarp
        for( unsigned thread=subwarp*subwarp_size; thread<subwarp_size*(subwarp+1); thread++ ) {
            if( !active(thread) )
                continue;

            unsigned data_size_coales = data_size;
            unsigned num_accesses = 1;

            if( space.get_type() == local_space || space.get_type() == param_space_local ) {
               // Local memory accesses >4B were split into 4B chunks
               if(data_size >= 4) {
                  data_size_coales = 4;
                  num_accesses = data_size/4;
               }
               // Otherwise keep the same data_size for sub-4B access to local memory
            }


            assert(num_accesses <= MAX_ACCESSES_PER_INSN_PER_THREAD);

            for(unsigned access=0; access<num_accesses; access++) {
                new_addr_type addr = m_per_scalar_thread[thread].memreqaddr[access];
                unsigned block_address = line_size_based_tag_func(addr,segment_size);
                unsigned chunk = (addr&127)/32; // which 32-byte chunk within in a 128-byte chunk does this thread access?
                transaction_info &info = subwarp_transactions[block_address];

                // can only write to one segment
                assert(block_address == line_size_based_tag_func(addr+data_size_coales-1,segment_size));

                info.chunks.set(chunk);
                info.active.set(thread);
                unsigned idx = (addr&127);
                for( unsigned i=0; i < data_size_coales; i++ )
                    info.bytes.set(idx+i);
            }
        }

        // step 2: reduce each transaction size, if possible
        std::map< new_addr_type, transaction_info >::iterator t;
        for( t=subwarp_transactions.begin(); t !=subwarp_transactions.end(); t++ ) {
            new_addr_type addr = t->first;
            const transaction_info &info = t->second;

            memory_coalescing_arch_13_reduce_and_send(is_write, access_type, info, addr, segment_size);

        }
    }
}

void warp_inst_t::memory_coalescing_arch_13_atomic( bool is_write, mem_access_type access_type )
{

   assert(space.get_type() == global_space); // Atomics allowed only for global memory

   // see the CUDA manual where it discusses coalescing rules before reading this
   unsigned segment_size = 0;
   unsigned warp_parts = 2;
   switch( data_size ) {
   case 1: segment_size = 32; break;
   case 2: segment_size = 64; break;
   case 4: case 8: case 16: segment_size = 128; break;
   }
   unsigned subwarp_size = m_config->warp_size / warp_parts;

   for( unsigned subwarp=0; subwarp <  warp_parts; subwarp++ ) {
       std::map<new_addr_type,std::list<transaction_info> > subwarp_transactions; // each block addr maps to a list of transactions

       // step 1: find all transactions generated by this subwarp
       for( unsigned thread=subwarp*subwarp_size; thread<subwarp_size*(subwarp+1); thread++ ) {
           if( !active(thread) )
               continue;

           new_addr_type addr = m_per_scalar_thread[thread].memreqaddr[0];
           unsigned block_address = line_size_based_tag_func(addr,segment_size);
           unsigned chunk = (addr&127)/32; // which 32-byte chunk within in a 128-byte chunk does this thread access?

           // can only write to one segment
           assert(block_address == line_size_based_tag_func(addr+data_size-1,segment_size));

           // Find a transaction that does not conflict with this thread's accesses
           bool new_transaction = true;
           std::list<transaction_info>::iterator it;
           transaction_info* info;
           for(it=subwarp_transactions[block_address].begin(); it!=subwarp_transactions[block_address].end(); it++) {
              unsigned idx = (addr&127);
              if(not it->test_bytes(idx,idx+data_size-1)) {
                 new_transaction = false;
                 info = &(*it);
                 break;
              }
           }
           if(new_transaction) {
              // Need a new transaction
              subwarp_transactions[block_address].push_back(transaction_info());
              info = &subwarp_transactions[block_address].back();
           }
           assert(info);

           info->chunks.set(chunk);
           info->active.set(thread);
           unsigned idx = (addr&127);
           for( unsigned i=0; i < data_size; i++ ) {
               assert(!info->bytes.test(idx+i));
               info->bytes.set(idx+i);
           }
       }

       // step 2: reduce each transaction size, if possible
       std::map< new_addr_type, std::list<transaction_info> >::iterator t_list;
       for( t_list=subwarp_transactions.begin(); t_list !=subwarp_transactions.end(); t_list++ ) {
           // For each block addr
           new_addr_type addr = t_list->first;
           const std::list<transaction_info>& transaction_list = t_list->second;

           std::list<transaction_info>::const_iterator t;
           for(t=transaction_list.begin(); t!=transaction_list.end(); t++) {
               // For each transaction
               const transaction_info &info = *t;
               memory_coalescing_arch_13_reduce_and_send(is_write, access_type, info, addr, segment_size);
           }
       }
   }
}

void warp_inst_t::memory_coalescing_arch_13_reduce_and_send( bool is_write, mem_access_type access_type, const transaction_info &info, new_addr_type addr, unsigned segment_size )
{
   assert( (addr & (segment_size-1)) == 0 );

   const std::bitset<4> &q = info.chunks;
   assert( q.count() >= 1 );
   std::bitset<2> h; // halves (used to check if 64 byte segment can be compressed into a single 32 byte segment)

   unsigned size=segment_size;
   if( segment_size == 128 ) {
       bool lower_half_used = q[0] || q[1];
       bool upper_half_used = q[2] || q[3];
       if( lower_half_used && !upper_half_used ) {
           // only lower 64 bytes used
           size = 64;
           if(q[0]) h.set(0);
           if(q[1]) h.set(1);
       } else if ( (!lower_half_used) && upper_half_used ) {
           // only upper 64 bytes used
           addr = addr+64;
           size = 64;
           if(q[2]) h.set(0);
           if(q[3]) h.set(1);
       } else {
           assert(lower_half_used && upper_half_used);
       }
   } else if( segment_size == 64 ) {
       // need to set halves
       if( (addr % 128) == 0 ) {
           if(q[0]) h.set(0);
           if(q[1]) h.set(1);
       } else {
           assert( (addr % 128) == 64 );
           if(q[2]) h.set(0);
           if(q[3]) h.set(1);
       }
   }
   if( size == 64 ) {
       bool lower_half_used = h[0];
       bool upper_half_used = h[1];
       if( lower_half_used && !upper_half_used ) {
           size = 32;
       } else if ( (!lower_half_used) && upper_half_used ) {
           addr = addr+32;
           size = 32;
       } else {
           assert(lower_half_used && upper_half_used);
       }
   }
   m_accessq.push_back( mem_access_t(access_type,addr,size,is_write,info.active,info.bytes) );
}

void warp_inst_t::completed( unsigned long long cycle ) const 
{
   unsigned long long latency = cycle - issue_cycle; 
   assert(latency <= cycle); // underflow detection 
   ptx_file_line_stats_add_latency(pc, latency * active_count());  
}


unsigned kernel_info_t::m_next_uid = 1;

kernel_info_t::kernel_info_t( dim3 gridDim, dim3 blockDim, class function_info *entry )
{
    m_kernel_entry=entry;
    m_grid_dim=gridDim;
    m_block_dim=blockDim;
    m_next_cta.x=0;
    m_next_cta.y=0;
    m_next_cta.z=0;
    m_next_tid=m_next_cta;
    m_num_cores_running=0;
    m_uid = m_next_uid++;
    m_param_mem = new memory_space_impl<8192>("param",64*1024);
}

kernel_info_t::~kernel_info_t()
{
    assert( m_active_threads.empty() );
    delete m_param_mem;
}

std::string kernel_info_t::name() const
{
    return m_kernel_entry->get_name();
}

#define MAX_VIRTUAL_ST_ENTRIES 32
#define MAX_VIRTUAL_RT_ENTRIES 32

simt_splits_table::simt_splits_table( unsigned wid, unsigned warpSize, const shader_core_config* config, const struct memory_config * mem_config, simt_tables * simt_table)
{
    m_warp_id=wid;
    m_warp_size = warpSize;
    m_active_split = (unsigned)-1;
    m_num_entries = 0;
    m_num_physical_entries = 0;
    m_num_transient_entries = 0;
    m_max_st_size = config->num_st_entries;
    m_warp_size = config->warp_size;
    m_config = config;
    m_mem_config = mem_config;
    m_response_st_entry=-1;
    m_spill_st_entry = warp_inst_t(config);
    m_fill_st_entry = warp_inst_t(config);
    m_spill_st_entry.clear();
    m_fill_st_entry.clear();
    m_simt_tables = simt_table;
    m_pending_recvg_entry = simt_splits_table_entry();
    reset();
}


void simt_splits_table::reset()
{
	m_num_entries = 0;
    m_splits_table.clear();
    for(unsigned i=0; i<MAX_VIRTUAL_ST_ENTRIES; i++){
    	m_splits_table[i]=simt_splits_table_entry();
    }
    while(!m_fifo_queue.empty()) m_fifo_queue.pop_front();
    while(!m_invalid_entries.empty()) m_invalid_entries.pop();
    for(int i=0; i<MAX_VIRTUAL_ST_ENTRIES; i++){
    	m_splits_table[i]=simt_splits_table_entry();
    }
	for(int i=MAX_VIRTUAL_ST_ENTRIES-1; i>=0; i--){
		m_invalid_entries.push(i);
	}

	for(int i=m_warp_size; i>=0; i--){
		m_available_v_id.push(i);
	}
}

void simt_splits_table::launch( address_type start_pc, const simt_mask_t &active_mask )
{
    reset();
    assert(!m_splits_table[0].m_valid);
    m_splits_table[0].m_pc = start_pc;
    m_splits_table[0].m_calldepth = 1;
    m_splits_table[0].m_active_mask = active_mask;
    m_splits_table[0].m_type = SPLITS_TABLE_TYPE_CALL;
    m_splits_table[0].m_valid=true;
    assert(m_invalid_entries.top()==0);
    m_invalid_entries.pop();
    assert(m_invalid_entries.size()>=0);
    m_active_split = 0;
    m_num_entries = 1;
    m_num_physical_entries = 1;
    assert((m_num_entries+m_invalid_entries.size())==MAX_VIRTUAL_ST_ENTRIES);
    m_fifo_queue.push_back(fifo_entry(0,(gpu_sim_cycle+gpu_tot_sim_cycle),m_fifo_queue.size()));
}

const simt_mask_t & simt_splits_table::get_active_mask()
{
    assert(m_splits_table.find(m_active_split)!=m_splits_table.end());
    assert(m_splits_table[m_active_split].m_valid);
    assert(m_splits_table[m_active_split].m_active_mask.any());
    return m_splits_table[m_active_split].m_active_mask;
}

const simt_mask_t & simt_splits_table::get_active_mask(unsigned num)
{
    assert(m_splits_table.find(num)!=m_splits_table.end());
    return m_splits_table[num].m_active_mask;
}

void simt_splits_table::get_pdom_splits_entry_info(unsigned num, unsigned *pc, unsigned *rpc )
{
    assert((m_splits_table.find(num)!=m_splits_table.end()) && m_splits_table[0].m_valid);
   *pc = m_splits_table[num].m_pc;
   *rpc = m_splits_table[num].m_recvg_pc;
}

void simt_splits_table::get_pdom_active_split_info(unsigned *pc, unsigned *rpc )
{
    assert((m_splits_table.find(m_active_split)!=m_splits_table.end()));
    assert(m_splits_table[m_active_split].m_valid);
   *pc = m_splits_table[m_active_split].m_pc;
   *rpc = m_splits_table[m_active_split].m_recvg_pc;
}

unsigned simt_splits_table::get_rpc(unsigned num)
{
    assert(m_splits_table.find(num)!=m_splits_table.end());
    return m_splits_table[num].m_recvg_pc;
}

void simt_splits_table::set_shader(shader_core_ctx* shader)
{
	m_shader=shader;
}

bool simt_splits_table::is_virtualized()
{
	return m_splits_table[m_active_split].m_virtual;
}

unsigned simt_splits_table::address_to_entry(warp_inst_t inst)
{
	if(!inst.empty()){
		unsigned wid = inst.warp_id();
		address_type addr = inst.pc;
		unsigned entry = (addr - BRU_VIR_START - (wid*m_config->warp_size)*MAX_BRU_VIR_PER_SPLIT)/(MAX_BRU_VIR_PER_SPLIT);
		return entry;
	}
	return (unsigned)-1;
}

bool simt_splits_table::blocked()
{
	return m_splits_table[m_active_split].m_blocked;
}


bool simt_splits_table::push_to_st_response_fifo(unsigned entry)
{
	if(m_response_st_entry==-1){
		m_response_st_entry=entry;
		return true;
	}
	return false;
}


unsigned simt_splits_table::get_replacement_candidate()
{
	unsigned entry = (unsigned)-1;
	for(unsigned i=m_fifo_queue.size()-1; i>=0; i--){
		fifo_entry replacement_candidate = m_fifo_queue[i];
		entry = replacement_candidate.m_st_entry;
		if(!m_splits_table[entry].m_virtual)
			break;
	}
	assert(entry!=(unsigned)-1);
	return entry;

}

bool simt_splits_table::spill_st_entry()
{
	if(!m_spill_st_entry.empty()) return false;

	assert(m_spill_st_entry.empty());
	/*
	 * - Choose replacement candidate
	 * - mark the entry as virtual
	 * - Create a warp_inst_t that contains the following information:
	 *    - store instruction
	 *    - for global memory
	 *    - address is a function of warp_id + spilt_id + base_address of split table
	 *    - the size
	 */
	unsigned entry_to_replace = get_replacement_candidate();
	//printf("virtualization of entry %u:%u:%u\n",entry_to_replace,m_num_physical_entries,m_num_entries);
	assert(!m_splits_table[entry_to_replace].m_virtual);
	m_splits_table[entry_to_replace].m_virtual = true;
	m_num_physical_entries--;
	assert(m_num_physical_entries!=(unsigned)-1);

	/*
	 * Size of ST entry:
	 * (PC-32bits, RPC-32bits, Active Mask-32-bits: : 12-bytes )
	 */

    address_type pc  = (m_warp_id*m_warp_size+entry_to_replace)*MAX_BRU_VIR_PER_SPLIT;
    address_type ppc = pc + BRU_VIR_START;
    unsigned nbytes=12;
    unsigned offset_in_block = pc & (m_config->m_L1D_config.get_line_sz()-1);
    if( (offset_in_block+nbytes) > m_config->m_L1I_config.get_line_sz() )
        nbytes = (m_config->m_L1D_config.get_line_sz()-offset_in_block);

    mem_access_t acc(BRU_ST_SPILL,ppc,nbytes,true);
    m_spill_st_entry.space = memory_space_t(local_space);
    m_spill_st_entry.cache_op = CACHE_WRITE_BACK;
    m_spill_st_entry.op = STORE_OP;
    m_spill_st_entry.mem_op = NOT_TEX;
	m_spill_st_entry.memory_op = bru_st_spill_request;
    m_spill_st_entry.pc = ppc;
	m_spill_st_entry.occupy();
	m_spill_st_entry.set_warp_id(m_warp_id);
	m_spill_st_entry.set_active(0);
	m_spill_st_entry.inject_mem_acccesses(acc);
	gpu_st_spills++;
    return true;
}



bool simt_splits_table::fill_st_entry(unsigned entry)
{
	if(!m_fill_st_entry.empty()) return false;
	/*
	 * - Check if the m_spill_st_entry is empty
	 *    - If not return false
	 *    - If yes, create a warp_inst_t that contains the following information:
	 *      - load instruction
	 *      - for global memory
	 *      - address is a function of warp_id + spilt_id + base_address of split table
	 *      - the size
	 */
	 address_type pc  = (m_warp_id*m_warp_size+entry)*MAX_BRU_VIR_PER_SPLIT;
	 address_type ppc = pc + BRU_VIR_START;
	 unsigned nbytes=12;
	 unsigned offset_in_block = pc & (m_config->m_L1D_config.get_line_sz()-1);
	 if( (offset_in_block+nbytes) > m_config->m_L1I_config.get_line_sz() )
		 nbytes = (m_config->m_L1D_config.get_line_sz()-offset_in_block);

	 mem_access_t acc(BRU_ST_FILL,ppc,nbytes,false);
	 m_fill_st_entry.space = memory_space_t(local_space);
	 m_fill_st_entry.cache_op = CACHE_ALL;
	 m_fill_st_entry.op = LOAD_OP;
	 m_fill_st_entry.mem_op = NOT_TEX;
	 m_fill_st_entry.memory_op = bru_st_fill_request;
	 m_fill_st_entry.pc = ppc;
	 m_fill_st_entry.inject_mem_acccesses(acc);
	 m_fill_st_entry.occupy();
	 m_fill_st_entry.set_warp_id(m_warp_id);
	 m_fill_st_entry.set_active(0);
	 m_splits_table[entry].m_transient=true;
	 m_num_transient_entries++;
	 gpu_st_fills++;
	 return true;
}


void simt_splits_table::cycle()
{
	/*
	 *  - Send the fill request through memory_cycle
	 *  - free m_spill_st_entry
	 */
	if(!m_spill_st_entry.empty()){
	    enum mem_stage_stall_type rc_fail = NO_RC_FAIL;
	    mem_stage_access_type type;
		bool done = true;
		done &= m_shader->memory_cycle(m_spill_st_entry,rc_fail, type);
		if( done ) {
			m_spill_st_entry.clear();
			m_spill_st_entry.clear_pending_mem_requests();
			unsigned entry = (m_spill_st_entry.pc - BRU_VIR_START - (m_spill_st_entry.get_warp_id()*m_config->warp_size)*MAX_BRU_VIR_PER_SPLIT)/(MAX_BRU_VIR_PER_SPLIT);
		}
	}

	if(!m_fill_st_entry.empty()){
	    enum mem_stage_stall_type rc_fail = NO_RC_FAIL;
	    mem_stage_access_type type;
		bool done = true;
		done &= m_shader->memory_cycle(m_fill_st_entry,rc_fail, type);
		if( done ) {
			m_fill_st_entry.clear();
			m_fill_st_entry.clear_pending_mem_requests();
			unsigned entry = (m_fill_st_entry.pc - BRU_VIR_START - (m_fill_st_entry.get_warp_id()*m_config->warp_size)*MAX_BRU_VIR_PER_SPLIT)/(MAX_BRU_VIR_PER_SPLIT);
		}
	}


	if(m_response_st_entry!=-1){
		if(m_num_physical_entries==m_max_st_size){
			bool spilled = spill_st_entry();
			if(spilled){
				assert(m_num_physical_entries<m_max_st_size);
				m_splits_table[m_response_st_entry].m_virtual=false;
				m_splits_table[m_response_st_entry].m_transient=false;
				m_num_transient_entries--;
				m_num_physical_entries++;
				m_response_st_entry=-1;
			}
		}else{
			assert(m_num_physical_entries<m_max_st_size);
			m_splits_table[m_response_st_entry].m_virtual=false;
			m_splits_table[m_response_st_entry].m_transient=false;
			m_num_transient_entries--;
			m_num_physical_entries++;
			m_response_st_entry=-1;
		}
	}

	if(m_pending_recvg_entry.m_valid){
		unsigned entry=insert_new_entry(m_pending_recvg_entry);
		if(entry!=(unsigned)-1){
			m_pending_recvg_entry.m_valid=false;
		}
	}


	if(m_splits_table[m_active_split].m_blocked || (!m_splits_table[m_active_split].m_valid && m_fifo_queue.size()>0))
		push_back();

	if(m_splits_table[m_active_split].m_virtual && !m_splits_table[m_active_split].m_transient)
		fill_st_entry(m_active_split);

}


unsigned simt_splits_table::insert_new_entry(simt_splits_table_entry entry, bool recvged)
{
	if(recvged){
		if(m_num_physical_entries == m_max_st_size){
			assert(!m_pending_recvg_entry.m_valid);
			m_pending_recvg_entry.m_valid=true;
			m_pending_recvg_entry.m_pc = entry.m_pc;
			m_pending_recvg_entry.m_recvg_pc = entry.m_recvg_pc;
			m_pending_recvg_entry.m_recvg_entry = entry.m_recvg_entry;
			m_pending_recvg_entry.m_active_mask = entry.m_active_mask;
			m_pending_recvg_entry.m_valid = true;
			m_pending_recvg_entry.m_virtual = false;
			m_pending_recvg_entry.m_type = entry.m_type;
			m_pending_recvg_entry.m_branch_div_cycle = entry.m_branch_div_cycle;
			return (unsigned)-1;
		}
	}else{
		if(m_num_physical_entries == m_max_st_size){
			 bool spilled = spill_st_entry();
			 if(!spilled){
				 return (unsigned)-1;
			 }
		}
	}
	assert(m_num_physical_entries < m_max_st_size);
	unsigned entry_num = m_invalid_entries.top();
	m_invalid_entries.pop();
	assert(m_invalid_entries.size()>=0);
	assert(!m_splits_table[entry_num].m_valid);
	m_splits_table[entry_num].m_pc = entry.m_pc;
	m_splits_table[entry_num].m_recvg_pc = entry.m_recvg_pc;
	m_splits_table[entry_num].m_recvg_entry = entry.m_recvg_entry;
	m_splits_table[entry_num].m_active_mask = entry.m_active_mask;
	m_splits_table[entry_num].m_valid = true;
	m_splits_table[entry_num].m_virtual = false;
	m_splits_table[entry_num].m_type = entry.m_type;
	m_splits_table[entry_num].m_branch_div_cycle = entry.m_branch_div_cycle;
	m_num_entries++;
	m_num_physical_entries++;
	assert((m_num_entries+m_invalid_entries.size())==MAX_VIRTUAL_ST_ENTRIES);
	assert(entry_num != (unsigned)-1);
	m_fifo_queue.push_back(fifo_entry(entry_num,(gpu_sim_cycle+gpu_tot_sim_cycle),m_fifo_queue.size()));

	return entry_num;
}

unsigned simt_splits_table::insert_new_entry(address_type pc, address_type rpc, unsigned rpc_entry, const simt_mask_t & tmp_active_mask,splits_table_entry_type type, bool recvged)
{

	if(recvged){
		if(m_num_physical_entries == m_max_st_size){
			assert(!m_pending_recvg_entry.m_valid);
			m_pending_recvg_entry.m_valid=true;
			m_pending_recvg_entry.m_pc = pc;
			m_pending_recvg_entry.m_recvg_pc = rpc;
			m_pending_recvg_entry.m_recvg_entry = rpc_entry;
			m_pending_recvg_entry.m_active_mask = tmp_active_mask;
			m_pending_recvg_entry.m_valid = true;
			m_pending_recvg_entry.m_virtual = false;
			m_pending_recvg_entry.m_type = type;
			m_pending_recvg_entry.m_branch_div_cycle = gpu_sim_cycle + gpu_tot_sim_cycle;
			return (unsigned)-1;
		}
	}else{
		if(m_num_physical_entries == m_max_st_size){
			 bool spilled = spill_st_entry();
			 if(!spilled){
				 return (unsigned)-1;
			 }
		}
	}

	unsigned entry = m_invalid_entries.top();
	m_invalid_entries.pop();
    if(m_invalid_entries.size()<0){
    	print(stdout);
    	abort();
    }
	assert(!m_splits_table[entry].m_valid);
	m_splits_table[entry].m_pc = pc;
	m_splits_table[entry].m_recvg_pc = rpc;
	m_splits_table[entry].m_recvg_entry = rpc_entry;
	m_splits_table[entry].m_active_mask = tmp_active_mask;
	m_splits_table[entry].m_valid = true;
	m_splits_table[entry].m_type=type;
	m_num_entries++;
	m_num_physical_entries++;
    assert((m_num_entries+m_invalid_entries.size())==MAX_VIRTUAL_ST_ENTRIES);
	assert(entry != (unsigned)-1);
	m_fifo_queue.push_back(fifo_entry(entry,(gpu_sim_cycle+gpu_tot_sim_cycle),m_fifo_queue.size()));
	return entry;
}


unsigned simt_reconvergence_table::insert_new_entry(address_type pc, address_type rpc, unsigned rpc_entry, const simt_mask_t & tmp_active_mask,splits_table_entry_type type)
{
	/*
	 * If all physical entries are occupied will need
	 * to spill an existing entry (spill one that has
	 * no associated non-virtual ST entry)
	 */
	if(m_num_physical_entries == m_max_rec_size){
		 bool spilled = spill_rec_entry();
		 /*
		  * This is guaranteed because branch instructions of a warp are scheduled
		  * only if there is a space to spill entries.
		  */
		 assert(spilled);
	}
	assert(tmp_active_mask.any());
	int entry_num = m_invalid_entries.top();
	m_invalid_entries.pop();
    assert(m_invalid_entries.size()>=0);
	assert(!m_recvg_table[entry_num].m_valid);
	m_recvg_table[entry_num].m_pc = pc;
	m_recvg_table[entry_num].m_recvg_pc = rpc;
	m_recvg_table[entry_num].m_recvg_entry = rpc_entry;
	m_recvg_table[entry_num].m_active_mask = tmp_active_mask;
	m_recvg_table[entry_num].m_pending_mask = tmp_active_mask;
	m_recvg_table[entry_num].m_valid = true;
	m_recvg_table[entry_num].m_branch_rec_cycle = gpu_sim_cycle+gpu_tot_sim_cycle;
	m_recvg_table[entry_num].m_type = type;
	m_num_entries++;
	m_num_physical_entries++;
	assert(entry_num != (unsigned)-1);
	return entry_num;
}

void simt_reconvergence_table::update_masks_upon_time_out(unsigned recvg_entry,const simt_mask_t & reconverged_mask)
{
	m_recvg_table[recvg_entry].m_active_mask  = m_recvg_table[recvg_entry].m_pending_mask;
}


unsigned simt_reconvergence_table::address_to_entry(warp_inst_t inst)
{
	if(!inst.empty()){
		unsigned wid = inst.warp_id();
		address_type addr = inst.pc;
		unsigned entry = (addr - BRU_VIR_START - (wid*m_config->warp_size)*MAX_BRU_VIR_PER_SPLIT - MAX_BRU_VIR_PER_SPLIT/2)/(MAX_BRU_VIR_PER_SPLIT);
		return entry;
	}
	return (unsigned)-1;
}


bool simt_reconvergence_table::update_pending_mask(unsigned recvg_entry,address_type recvg_pc,const simt_mask_t & tmp_active_mask, bool &suspended)
{
	assert(m_recvg_table[recvg_entry].m_pc == recvg_pc);
	assert(m_recvg_table[recvg_entry].m_valid);
	assert(m_recvg_table[recvg_entry].m_pending_mask.any());
	const simt_mask_t & reconverged_mask = (~m_recvg_table[recvg_entry].m_pending_mask) & (m_recvg_table[recvg_entry].m_active_mask);
	unsigned long long diff = ((gpu_sim_cycle+gpu_tot_sim_cycle) - m_recvg_table[recvg_entry].m_branch_rec_cycle);
	if(reconverged_mask.any()){
		if(diff>max_recvg_time){
			max_recvg_time=diff;
		}
	}

	if(m_recvg_table[recvg_entry].m_virtual){
		suspended = true;
		assert(!m_pending_update_entry.m_valid);
		if(!m_recvg_table[recvg_entry].m_transient){
			m_pending_update_entry.m_valid=true;
			m_pending_update_entry.m_active_mask = tmp_active_mask;
			m_pending_update_entry.m_branch_rec_cycle = gpu_sim_cycle+gpu_tot_sim_cycle;
			m_pending_update_entry.m_recvg_entry=recvg_entry;
			m_pending_update_entry.m_transient=false;

		}
	}else{
		m_recvg_table[recvg_entry].m_branch_rec_cycle = gpu_sim_cycle+gpu_tot_sim_cycle;
		m_recvg_table[recvg_entry].m_pending_mask = m_recvg_table[recvg_entry].m_pending_mask & ~ tmp_active_mask;
		if(!m_recvg_table[recvg_entry].m_pending_mask.any()){
			invalidate(recvg_entry);
			return true;
		}
	}
	return false;
}


void simt_splits_table::push_back()
{
	   fifo_entry cur_active_split = m_fifo_queue.front();
	   assert(cur_active_split.m_st_entry==m_active_split);
	   m_fifo_queue.pop_front();
	   cur_active_split.update_insertion_cycle(gpu_sim_cycle+gpu_tot_sim_cycle,m_fifo_queue.size());
	   m_fifo_queue.push_back(cur_active_split);
	   fifo_entry new_active_split = m_fifo_queue.front();
	   m_active_split = new_active_split.m_st_entry;

	   if(m_splits_table[m_active_split].m_virtual){
		  if(!m_splits_table[m_active_split].m_transient && !m_splits_table[m_active_split].m_blocked){
			  bool fill_request_sent = fill_st_entry(m_active_split);
		  }
	   }
}

void simt_splits_table::update_active_entry()
{
	if(m_fifo_queue.size()>0){
		fifo_entry new_active_entry = m_fifo_queue.front();
		m_active_split = new_active_entry.m_st_entry;
		if(new_active_entry.m_blocked || m_splits_table[m_active_split].m_virtual) push_back();
		assert(m_num_entries>0);
		assert((m_splits_table.find(m_active_split)!=m_splits_table.end()));
		assert(m_splits_table[m_active_split].m_valid);
		assert(m_splits_table[m_active_split].m_active_mask.any());
	  }
}


void simt_splits_table::invalidate()
{
   if(!m_splits_table[m_active_split].m_valid) return;
   assert((m_splits_table.find(m_active_split)!=m_splits_table.end()));
   assert(m_splits_table[m_active_split].m_valid);
   assert(m_fifo_queue.front().m_st_entry==m_active_split);
   assert(!m_splits_table[m_active_split].m_virtual);

   m_splits_table[m_active_split].m_valid=false;
   m_fifo_queue.pop_front();
   m_num_entries--;
   m_num_physical_entries--;
   assert(m_num_physical_entries!=(unsigned)-1);
   m_invalid_entries.push(m_active_split);
   assert((m_num_entries+m_invalid_entries.size())==MAX_VIRTUAL_ST_ENTRIES);
}
void simt_splits_table::update_pc(address_type new_pc)
{
	assert((m_splits_table.find(m_active_split)!=m_splits_table.end()));
	assert(m_splits_table[m_active_split].m_valid);
	assert(m_fifo_queue.front().m_st_entry==m_active_split);
	m_splits_table[m_active_split].m_pc=new_pc;
}

void simt_splits_table::set_to_blocked()
{
	m_splits_table[m_active_split].m_blocked=true;
	//print(stdout);
}

void simt_splits_table::unset_blocked()
{
	m_splits_table[m_active_split].m_blocked=false;
}

void simt_splits_table::unset_blocked(unsigned entry)
{
	m_splits_table[entry].m_blocked=false;
}

void simt_splits_table::release_blocked()
{
	for(unsigned i=0; i<MAX_VIRTUAL_ST_ENTRIES; i++){
		if(m_splits_table[i].m_valid)
			unset_blocked(i);
	}

	for(unsigned i=0; i< m_fifo_queue.size(); i++){
		m_fifo_queue[i].m_blocked=false;
	}

}

bool simt_splits_table::is_blocked_or_virtual()
{
	bool blocked = true;
	for(unsigned i=0; i<MAX_VIRTUAL_ST_ENTRIES; i++){
		if(m_splits_table[i].m_valid )
			blocked &= (m_splits_table[i].m_blocked || m_splits_table[i].m_virtual) ;
	}
	return (blocked && m_num_entries>0);

}

bool simt_splits_table::is_virtual()
{
	bool virtualized = true;
	for(unsigned i=0; i<MAX_VIRTUAL_ST_ENTRIES; i++){
		if(m_splits_table[i].m_valid )
			virtualized &= (m_splits_table[i].m_virtual) ;
	}
	return (virtualized && m_num_entries>0);

}

bool simt_splits_table::is_blocked()
{
	bool blocked = true;
	for(unsigned i=0; i<MAX_VIRTUAL_ST_ENTRIES; i++){
		if(m_splits_table[i].m_valid )
			blocked &= (m_splits_table[i].m_blocked);
	}
	return (blocked && m_num_entries>0);
}

bool simt_splits_table::split_reaches_barrier(address_type pc)
{
	m_fifo_queue.front().m_blocked = true;
	set_to_blocked();
	update_pc(pc);
	return is_blocked();
}


unsigned simt_splits_table::get_rpc()
{
    assert(m_splits_table.find(m_active_split)!=m_splits_table.end());
    return m_splits_table[m_active_split].m_recvg_pc;
}

unsigned simt_splits_table::get_rpc_entry()
{
    assert(m_splits_table.find(m_active_split)!=m_splits_table.end());
    return m_splits_table[m_active_split].m_recvg_entry;
}


unsigned simt_splits_table::get_pc()
{
    assert(m_splits_table.find(m_active_split)!=m_splits_table.end());
    return m_splits_table[m_active_split].m_pc;
}

splits_table_entry_type simt_splits_table::get_type()
{
    assert(m_splits_table.find(m_active_split)!=m_splits_table.end());
    return m_splits_table[m_active_split].m_type;
}

bool simt_splits_table::valid()
{
	//printf("valid m_active_split=%u\n",m_active_split);
    assert(m_splits_table.find(m_active_split)!=m_splits_table.end());
    return m_splits_table[m_active_split].m_valid;
}



void simt_splits_table::print (FILE *fout)
{
	printf("max of physical entries=%u\n",m_max_st_size);
	printf("num of physical entries=%u\n",m_num_physical_entries);
	printf("isBlocked? %u\n",is_blocked());
	fprintf(fout, "fifo- f: %02d\n", m_fifo_queue.front().m_st_entry);
	fprintf(fout, "fifo- b: %02d\n", m_fifo_queue.back().m_st_entry);
	fprintf(fout, "fifo-sz: %02d\n", m_fifo_queue.size());
	printf("active entry=%u\n",m_active_split);
	printf("Spill Entry %u\n",address_to_entry(m_spill_st_entry));
	printf("fill Entry %u\n",address_to_entry(m_fill_st_entry));
	printf("Pending Recvg Entry valid: %u\n",m_pending_recvg_entry.m_valid);
	printf("Response Entry: %u\n",m_response_st_entry);
	for ( unsigned k=0; k < MAX_VIRTUAL_ST_ENTRIES; k++ ) {
        simt_splits_table_entry splits_table_entry = m_splits_table[k];
        if(!splits_table_entry.m_valid) continue;
        if(splits_table_entry.m_virtual) printf("Virtual: ");
        if(splits_table_entry.m_transient) printf("Transient: ");
        if(splits_table_entry.m_blocked) printf("Blocked: ");
        fprintf(fout, "       %1u ", k );
        for (unsigned j=0; j<m_warp_size; j++)
            fprintf(fout, "%c", (splits_table_entry.m_active_mask.test(j)?'1':'0') );
        fprintf(fout, " pc: 0x%03x", splits_table_entry.m_pc );

        if ( splits_table_entry.m_recvg_pc == (unsigned)-1 ) {
            fprintf(fout," rp: ----      %s", (splits_table_entry.m_valid==true?" V ":" N "));
        } else {
            fprintf(fout," rp: 0x%03x    %s", splits_table_entry.m_recvg_pc, (splits_table_entry.m_valid==true?" V ":" N "));
        }

        ptx_print_insn( splits_table_entry.m_pc, fout );
        fprintf(fout,"\n");
    }
}


simt_reconvergence_table::simt_reconvergence_table( unsigned wid,  unsigned warpSize,  const shader_core_config* config, const struct memory_config * m_mem_config, simt_tables * simt_table)
{
    m_warp_id=wid;
    m_warp_size = warpSize;
    m_active_reconvergence = (unsigned)-1;
    m_num_entries = 0;
    m_num_physical_entries = 0;
    m_num_transient_entries=0;
    m_max_rec_size = config->num_rec_entries;
    m_simt_tables = simt_table;
	m_spill_rec_entry = warp_inst_t(config);
	m_fill_rec_entry = warp_inst_t(config);
	m_pending_update_entry = simt_reconvergence_table_entry();
	m_response_rec_entry = -1;
	m_config = config;
    reset();
    assert((m_num_entries+m_invalid_entries.size())==MAX_VIRTUAL_RT_ENTRIES);
}

void simt_reconvergence_table::reset()
{
	m_num_entries = 0;
	m_recvg_table.clear();
    while(!m_invalid_entries.empty()) m_invalid_entries.pop();
    for(int i=0; i<MAX_VIRTUAL_RT_ENTRIES; i++){
    	m_recvg_table[i]=simt_reconvergence_table_entry();
    }
	for(int i=MAX_VIRTUAL_RT_ENTRIES-1; i>=0; i--){
		m_invalid_entries.push(i);
	}
    assert((m_num_entries+m_invalid_entries.size())==MAX_VIRTUAL_RT_ENTRIES);

}
bool simt_reconvergence_table::push_to_rt_response_fifo(unsigned entry)
{
	if(m_response_rec_entry==-1){
		m_response_rec_entry=entry;
		return true;
	}
	return false;
}

unsigned simt_reconvergence_table::get_replacement_candidate()
{
	unsigned oldest_index = (unsigned)-1;
	unsigned long long oldest_update = 0;
	for(int i=0; i<MAX_VIRTUAL_RT_ENTRIES;i++){
		if(m_recvg_table[i].m_valid){
			if(!m_recvg_table[i].m_virtual){
				if(oldest_update==0){
					oldest_index = i;
					oldest_update=m_recvg_table[i].m_branch_rec_cycle;
				}else{
					if(m_recvg_table[i].m_branch_rec_cycle>oldest_update){
						oldest_index = i;
						oldest_update=m_recvg_table[i].m_branch_rec_cycle;
					}
				}
			}
		}
	}
	assert(oldest_index!=(unsigned)-1);
	return oldest_index;

}


bool simt_reconvergence_table::fill_rec_entry(unsigned entry)
{
	if(!m_fill_rec_entry.empty()) return false;
	/*
	 * - Check if the m_spill_st_entry is empty
	 *    - If not return false
	 *    - If yes, create a warp_inst_t that contains the following information:
	 *      - load instruction
	 *      - for global memory
	 *      - address is a function of warp_id + spilt_id + base_address of split table
	 *      - the size
	 */
	 address_type pc  = (m_warp_id*m_warp_size+entry)*MAX_BRU_VIR_PER_SPLIT + (MAX_BRU_VIR_PER_SPLIT/2);
	 address_type ppc = pc + BRU_VIR_START;
	 unsigned nbytes=16;
	 unsigned offset_in_block = pc & (m_config->m_L1D_config.get_line_sz()-1);
	 if( (offset_in_block+nbytes) > m_config->m_L1I_config.get_line_sz() )
		 nbytes = (m_config->m_L1D_config.get_line_sz()-offset_in_block);

	 mem_access_t acc(BRU_RT_FILL,ppc,nbytes,false);
	 m_fill_rec_entry.space = memory_space_t(local_space);
	 m_fill_rec_entry.cache_op = CACHE_ALL;
	 m_fill_rec_entry.op = LOAD_OP;
	 m_fill_rec_entry.mem_op = NOT_TEX;
	 m_fill_rec_entry.memory_op = bru_rt_fill_request;
	 m_fill_rec_entry.pc = ppc;
	 m_fill_rec_entry.inject_mem_acccesses(acc);
	 m_fill_rec_entry.occupy();
	 m_fill_rec_entry.set_warp_id(m_warp_id);
	 m_fill_rec_entry.set_active(0);
	 m_recvg_table[entry].m_transient=true;
	 m_num_transient_entries++;
	 gpu_rt_fills++;
	 return true;


}


bool simt_reconvergence_table::spill_rec_entry()
{
	if(!m_spill_rec_entry.empty()) return false;
	assert(m_spill_rec_entry.empty());
	/*
	 * - Choose replacement candidate
	 * - mark the entry as virtual
	 * - Create a warp_inst_t that contains the following information:
	 *    - store instruction
	 *    - for global memory
	 *    - address is a function of warp_id + spilt_id + base_address of split table
	 *    - the size
	 *  - Attempt to send the instruction through the memory cycle (if sent - clear m_spill_st_entry)
	 */
	unsigned entry_to_replace = get_replacement_candidate();
	printf("virtualization of entry %u\n",entry_to_replace);
	m_recvg_table[entry_to_replace].m_virtual = true;
	m_num_physical_entries--;
	assert(m_num_physical_entries!=(unsigned)-1);
	/*
	 * Size of ST entry:
	 * (PC-32bits, RPC-32bits, Active Mask-32-bits: RPC-entry-5bits, vali/virtual/blocked 3-bits: 12-bytes )
	 */

    address_type pc  = (m_warp_id*m_warp_size+entry_to_replace)*MAX_BRU_VIR_PER_SPLIT+(MAX_BRU_VIR_PER_SPLIT/2);
    address_type ppc = pc + BRU_VIR_START;
    unsigned nbytes=16;
    unsigned offset_in_block = pc & (m_config->m_L1D_config.get_line_sz()-1);
    if( (offset_in_block+nbytes) > m_config->m_L1I_config.get_line_sz() )
        nbytes = (m_config->m_L1D_config.get_line_sz()-offset_in_block);

    mem_access_t acc(BRU_RT_SPILL,ppc,nbytes,true);
    m_spill_rec_entry.space = memory_space_t(local_space);
    m_spill_rec_entry.cache_op = CACHE_WRITE_BACK;
    m_spill_rec_entry.op = STORE_OP;
    m_spill_rec_entry.mem_op = NOT_TEX;
	m_spill_rec_entry.memory_op = bru_rt_spill_request;
    m_spill_rec_entry.pc = ppc;
	m_spill_rec_entry.occupy();
	m_spill_rec_entry.set_warp_id(m_warp_id);
	m_spill_rec_entry.set_active(0);
	m_spill_rec_entry.inject_mem_acccesses(acc);
	gpu_rt_spills++;
    return true;


}


void simt_reconvergence_table::cycle()
{
	if(m_pending_update_entry.m_valid && !m_pending_update_entry.m_transient){
		bool sent=fill_rec_entry(m_pending_update_entry.m_recvg_entry);
		if(sent){
			m_pending_update_entry.m_transient=true;
		}
	}


	if(!m_spill_rec_entry.empty()){
	    enum mem_stage_stall_type rc_fail = NO_RC_FAIL;
	    mem_stage_access_type type;
		bool done = true;
		done &= m_shader->memory_cycle(m_spill_rec_entry,rc_fail, type);
		if( done ) {
			m_spill_rec_entry.clear();
			m_spill_rec_entry.clear_pending_mem_requests();
			unsigned entry = (m_spill_rec_entry.pc - BRU_VIR_START - (m_spill_rec_entry.get_warp_id()*m_config->warp_size)*MAX_BRU_VIR_PER_SPLIT)/(MAX_BRU_VIR_PER_SPLIT);
		}
	}

	if(!m_fill_rec_entry.empty()){
	    enum mem_stage_stall_type rc_fail = NO_RC_FAIL;
	    mem_stage_access_type type;
		bool done = true;
		done &= m_shader->memory_cycle(m_fill_rec_entry,rc_fail, type);
		if( done ) {
			m_fill_rec_entry.clear();
			m_fill_rec_entry.clear_pending_mem_requests();
			unsigned entry = (m_fill_rec_entry.pc - BRU_VIR_START - (m_fill_rec_entry.get_warp_id()*m_config->warp_size)*MAX_BRU_VIR_PER_SPLIT)/(MAX_BRU_VIR_PER_SPLIT);
		}
	}


	if(m_response_rec_entry!=-1){
		if(m_pending_update_entry.m_recvg_entry==m_response_rec_entry){
			assert(m_pending_update_entry.m_valid);
			simt_mask_t test = m_recvg_table[m_response_rec_entry].m_pending_mask & ~ m_pending_update_entry.m_active_mask;
			bool converged = !test.any();
			bool spacefound = !m_simt_tables->is_pending_reconvergence() || (m_simt_tables->st_space_available());
			if(m_num_physical_entries==m_max_rec_size){
				bool spilled = spill_rec_entry();
				if(spilled && (!converged || (converged && spacefound))){
					assert(m_num_physical_entries<=m_max_rec_size);
					m_recvg_table[m_response_rec_entry].m_virtual=false;
					m_recvg_table[m_response_rec_entry].m_transient=false;
					m_recvg_table[m_response_rec_entry].m_pending_mask=
					m_recvg_table[m_response_rec_entry].m_pending_mask = m_recvg_table[m_response_rec_entry].m_pending_mask & ~ m_pending_update_entry.m_active_mask;
					m_recvg_table[m_response_rec_entry].m_branch_rec_cycle = m_pending_update_entry.m_branch_rec_cycle;
					m_pending_update_entry.m_valid=false;
					m_num_transient_entries--;
					m_num_physical_entries++;
					if(converged){
						invalidate(m_response_rec_entry);
						simt_mask_t  active_mask =  get_active_mask(m_response_rec_entry);
						address_type pc = get_pc(m_response_rec_entry);
						address_type rpc = get_rpc(m_response_rec_entry);
						unsigned rpc_entry = get_rpc_entry(m_response_rec_entry);
						splits_table_entry_type type = get_type(m_response_rec_entry);
						m_simt_tables->insert_st_entry(pc,rpc,rpc_entry,active_mask,type,true);
					}
					m_response_rec_entry=-1;
				}
			}else{
				if((!converged || (converged && spacefound))){
					assert(m_num_physical_entries<m_max_rec_size);
					m_recvg_table[m_response_rec_entry].m_virtual=false;
					m_recvg_table[m_response_rec_entry].m_transient=false;
					m_recvg_table[m_response_rec_entry].m_pending_mask=
					m_recvg_table[m_response_rec_entry].m_pending_mask = m_recvg_table[m_response_rec_entry].m_pending_mask & ~ m_pending_update_entry.m_active_mask;
					m_recvg_table[m_response_rec_entry].m_branch_rec_cycle = m_pending_update_entry.m_branch_rec_cycle;
					m_pending_update_entry.m_valid=false;
					m_num_transient_entries--;
					m_num_physical_entries++;
					if(converged){
						invalidate(m_response_rec_entry);
						simt_mask_t  active_mask =  get_active_mask(m_response_rec_entry);
						address_type pc = get_pc(m_response_rec_entry);
						address_type rpc = get_rpc(m_response_rec_entry);
						unsigned rpc_entry = get_rpc_entry(m_response_rec_entry);
						splits_table_entry_type type = get_type(m_response_rec_entry);
						m_simt_tables->insert_st_entry(pc,rpc,rpc_entry,active_mask,type,true);
					}
					m_response_rec_entry=-1;
				}
			}
		}else{
	    	unsigned long long diff = ((gpu_sim_cycle+gpu_tot_sim_cycle) - m_recvg_table[m_response_rec_entry].m_branch_rec_cycle);
			if(diff>m_config->rec_time_out){
		    	const simt_mask_t & reconverged_mask = (~m_recvg_table[m_response_rec_entry].m_pending_mask) & (m_recvg_table[m_response_rec_entry].m_active_mask);

				triggered_timeouts++;
				//insert an entry in the ST table with reconverged_mask
				address_type pc = m_recvg_table[m_response_rec_entry].m_pc;
				address_type rpc = m_recvg_table[m_response_rec_entry].m_recvg_pc;
				unsigned rpc_entry = m_recvg_table[m_response_rec_entry].m_recvg_entry;
				splits_table_entry_type type = m_recvg_table[m_response_rec_entry].m_type;
				m_simt_tables->insert_st_entry(pc,rpc,rpc_entry,reconverged_mask,type,true);
				//update the reconvergence mask of this entry
				update_masks_upon_time_out(m_response_rec_entry,reconverged_mask);
				set_rec_cycle(m_response_rec_entry,(gpu_sim_cycle+gpu_tot_sim_cycle));

			}
		}
	}


}


const simt_mask_t & simt_reconvergence_table::get_active_mask()
{
    assert(	m_recvg_table.find(m_active_reconvergence)!= m_recvg_table.end());
    return 	m_recvg_table[m_active_reconvergence].m_active_mask;
}

const simt_mask_t & simt_reconvergence_table::get_active_mask(unsigned num)
{
    assert(m_recvg_table.find(num)!=m_recvg_table.end());
    return m_recvg_table[num].m_active_mask;
}

simt_reconvergence_table_entry  simt_reconvergence_table::get_recvg_entry(unsigned num)
{
	return m_recvg_table[num];
}


void simt_reconvergence_table::get_recvg_entry_info(unsigned num, unsigned *pc, unsigned *rpc )
{
    assert((m_recvg_table.find(num)!=m_recvg_table.end()) && m_recvg_table[0].m_valid);
   *pc = m_recvg_table[num].m_pc;
   *rpc = m_recvg_table[num].m_recvg_pc;
}

void simt_reconvergence_table::get_active_recvg_info(unsigned *pc, unsigned *rpc )
{
    assert((m_recvg_table.find(m_active_reconvergence)!=m_recvg_table.end()) && m_recvg_table[0].m_valid);
   *pc = m_recvg_table[m_active_reconvergence].m_pc;
   *rpc = m_recvg_table[m_active_reconvergence].m_recvg_pc;
}

unsigned simt_reconvergence_table::get_rpc_entry()
{
    assert(m_recvg_table.find(m_active_reconvergence)!=m_recvg_table.end());
    return m_recvg_table[m_active_reconvergence].m_recvg_entry;
}


unsigned simt_reconvergence_table::get_rpc_entry(unsigned num)
{
    assert(m_recvg_table.find(num)!=m_recvg_table.end());
    return m_recvg_table[num].m_recvg_entry;
}

splits_table_entry_type simt_reconvergence_table::get_type(unsigned num)
{
    assert(m_recvg_table.find(num)!=m_recvg_table.end());
    return m_recvg_table[num].m_type;
}



unsigned simt_reconvergence_table::get_rpc()
{
    assert(m_recvg_table.find(m_active_reconvergence)!=m_recvg_table.end());
    return m_recvg_table[m_active_reconvergence].m_recvg_pc;
}


unsigned simt_reconvergence_table::get_rpc(unsigned num)
{
    assert(m_recvg_table.find(num)!=m_recvg_table.end());
    return m_recvg_table[num].m_recvg_pc;
}


unsigned simt_reconvergence_table::get_pc()
{
    assert(m_recvg_table.find(m_active_reconvergence)!=m_recvg_table.end());
    return m_recvg_table[m_active_reconvergence].m_pc;
}



unsigned simt_reconvergence_table::get_pc(unsigned num)
{
    assert(m_recvg_table.find(num)!=m_recvg_table.end());
    return m_recvg_table[num].m_pc;
}




void simt_reconvergence_table::invalidate()
{
   assert((m_recvg_table.find(m_active_reconvergence)!=m_recvg_table.end()) && m_recvg_table[m_active_reconvergence].m_valid);
   m_recvg_table[m_active_reconvergence].m_valid=false;
   m_invalid_entries.push(m_active_reconvergence);
   m_num_entries--;
   m_num_physical_entries--;
   assert(m_num_physical_entries!=(unsigned)-1);
   assert((m_num_entries+m_invalid_entries.size())==MAX_VIRTUAL_RT_ENTRIES);
}

void simt_reconvergence_table::invalidate(unsigned num)
{
   assert((m_recvg_table.find(num)!=m_recvg_table.end()) && m_recvg_table[num].m_valid);
   m_recvg_table[num].m_valid=false;
   m_invalid_entries.push(num);
   m_num_entries--;
   m_num_physical_entries--;
   assert(m_num_physical_entries!=(unsigned)-1);
   assert((m_num_entries+m_invalid_entries.size())==MAX_VIRTUAL_RT_ENTRIES);
}

void simt_reconvergence_table::set_rec_cycle(unsigned rec_entry,unsigned long long time)
{
	 m_recvg_table[rec_entry].m_branch_rec_cycle = time;
}



void simt_reconvergence_table::print (FILE *fout)
{
	printf("max of physical entries=%u\n",m_max_rec_size);
	printf("num of physical entries=%u\n",m_num_physical_entries);
	printf("Spill Entry %u\n",address_to_entry(m_spill_rec_entry));
	printf("fill Entry %u\n",address_to_entry(m_fill_rec_entry));
	printf("Pending Update Entry valid: %u\n",m_pending_update_entry.m_valid);
	printf("Response Entry: %u\n",m_response_rec_entry);
	for ( unsigned k=0; k <MAX_VIRTUAL_RT_ENTRIES; k++ ) {
        simt_reconvergence_table_entry recvg_table_entry = m_recvg_table[k];
        if(!recvg_table_entry.m_valid) continue;
        fprintf(fout, "        %1u ", k );
        for (unsigned j=0; j<m_warp_size; j++)
            fprintf(fout, "%c", (recvg_table_entry.m_active_mask.test(j)?'1':'0') );
        fprintf(fout,"  ");
        for (unsigned j=0; j<m_warp_size; j++)
            fprintf(fout, "%c", (recvg_table_entry.m_pending_mask.test(j)?'1':'0') );

        fprintf(fout, " pc: 0x%03x", recvg_table_entry.m_pc );
        if ( recvg_table_entry.m_recvg_pc == (unsigned)-1 ) {
            fprintf(fout," rp: ----       %s", (recvg_table_entry.m_valid==true?" V ":" N "));
        } else {
            fprintf(fout," rp: 0x%03x     %s", recvg_table_entry.m_recvg_pc, (recvg_table_entry.m_valid==true?" V ":" N "));
        }
        ptx_print_insn( recvg_table_entry.m_pc, fout );
        fprintf(fout,"\n");
    }
}


simt_tables::simt_tables( unsigned wid,  unsigned warpSize, const shader_core_config *config, const struct memory_config *mem_config)
{
    m_warp_id=wid;
    m_warp_size = warpSize;
	m_simt_splits_table = new simt_splits_table(wid,warpSize,config,mem_config,this);
	m_simt_recvg_table = new simt_reconvergence_table(wid,warpSize,config,mem_config,this);
	m_config = config;
	m_mem_config = mem_config;
}

void simt_tables::reset()
{
	m_simt_splits_table->reset();
	m_simt_recvg_table->reset();
	m_simt_splits_table->release_blocked();
}

void simt_tables::launch( address_type start_pc, const simt_mask_t &active_mask )
{
	m_simt_splits_table->launch(start_pc,active_mask);
}


unsigned simt_splits_table::check_simt_splits_table()
{
	unsigned count=0;
	for(unsigned i=0; i<MAX_VIRTUAL_ST_ENTRIES;i++){
		if(m_splits_table[i].m_valid){
	        for (unsigned j=0; j<m_warp_size; j++)
	            if(m_splits_table[i].m_active_mask.test(j)){
	            	count++;
	            }
		}
	}
	return count;
}

unsigned simt_reconvergence_table::check_simt_reconvergence_table()
{
	unsigned count=0;
	for(unsigned i=0; i<MAX_VIRTUAL_RT_ENTRIES;i++){
		if(m_recvg_table[i].m_valid){
	        for (unsigned j=0; j<m_warp_size; j++)
	            if(!m_recvg_table[i].m_pending_mask.test(j) && m_recvg_table[i].m_active_mask.test(j)){
	            	count++;
	            }
		}
	}
	return count;
}


void simt_tables::check_time_out()
{

	for ( unsigned k=0; k < m_config->num_rec_entries; k++ ) {
        simt_reconvergence_table_entry recvg_table_entry = m_simt_recvg_table->get_recvg_entry(k);
        if(!recvg_table_entry.m_valid) continue;
    	const simt_mask_t & reconverged_mask = (~recvg_table_entry.m_pending_mask) & (recvg_table_entry.m_active_mask);

    	unsigned long long diff = ((gpu_sim_cycle+gpu_tot_sim_cycle) - recvg_table_entry.m_branch_rec_cycle);
		if(diff>m_config->rec_time_out){
			if(recvg_table_entry.m_virtual){
				m_simt_recvg_table->fill_rec_entry(k);
			}else{
				if(reconverged_mask.any()){
					triggered_timeouts++;
					//insert an entry in the ST table with reconverged_mask
					address_type pc = recvg_table_entry.m_pc;
					address_type rpc = recvg_table_entry.m_recvg_pc;
					unsigned rpc_entry = recvg_table_entry.m_recvg_entry;
					splits_table_entry_type type = recvg_table_entry.m_type;
					m_simt_splits_table->insert_new_entry(pc,rpc,rpc_entry,reconverged_mask,type,true);
					//update the reconvergence mask of this entry
					m_simt_recvg_table->update_masks_upon_time_out(k,reconverged_mask);
					m_simt_recvg_table->set_rec_cycle(k,(gpu_sim_cycle+gpu_tot_sim_cycle));
				}
			}
    	}
    }
}

bool simt_tables::split_reaches_barrier(address_type pc)
{
	return m_simt_splits_table->split_reaches_barrier(pc);
}


void simt_tables::release_barrier()
{
	m_simt_splits_table->release_blocked();
}

bool simt_tables::is_virtualized()
{
	return m_simt_splits_table->is_virtualized();
}

bool simt_tables::is_pending_reconvergence()
{
	return m_simt_splits_table->is_pending_reconvergence() && m_simt_recvg_table->is_pending_update();
}

bool simt_tables::st_space_available()
{
	return m_simt_splits_table->st_space_available();
}


bool simt_tables::blocked()
{
	return m_simt_splits_table->blocked();
}

bool simt_tables::is_blocked()
{
	return m_simt_splits_table->is_blocked();
}


bool simt_tables::valid()
{
	return m_simt_splits_table->valid();
}


bool simt_tables::push_to_rt_response_fifo(unsigned entry)
{
	return m_simt_recvg_table->push_to_rt_response_fifo(entry);
}

bool simt_tables::push_to_st_response_fifo(unsigned entry)
{
	return m_simt_splits_table->push_to_st_response_fifo(entry);
}

void simt_tables::set_shader(shader_core_ctx* shader)
{
	m_shader=shader;
	m_simt_splits_table->set_shader(shader);
	m_simt_recvg_table->set_shader(shader);
}


void simt_tables::push_back()
{
	m_simt_splits_table->push_back();
}



void simt_tables::check_simt_tables()
{

	unsigned running = m_simt_splits_table->check_simt_splits_table();
	unsigned converged = m_simt_recvg_table->check_simt_reconvergence_table();
	if(running+converged>32){
		printf("running=%u\n",running);
		printf("converged=%u\n",converged);
		abort();
	}
}

void simt_tables::update( simt_mask_t &thread_done, addr_vector_t &next_pc, address_type recvg_pc, op_type next_inst_op,unsigned next_inst_size, address_type next_inst_pc)
{
    check_simt_tables();

    simt_mask_t  top_active_mask = m_simt_splits_table->get_active_mask();
    simt_mask_t  top_active_mask_keep = m_simt_splits_table->get_active_mask();

    address_type top_recvg_pc = m_simt_splits_table->get_rpc();
    unsigned top_recvg_entry = m_simt_splits_table->get_rpc_entry();
    address_type top_pc = m_simt_splits_table->get_pc();
    assert(top_pc==next_inst_pc);
    assert(top_active_mask.any());
    const address_type null_pc = -1;
    bool warp_diverged = false;
    address_type new_recvg_pc = null_pc;
    unsigned num_divergent_paths=0;
    unsigned new_recvg_entry=top_recvg_entry;
    splits_table_entry_type top_type =m_simt_splits_table->get_type();

    std::map<address_type,simt_mask_t> divergent_paths;
    bool invalidate = false;

    while (top_active_mask.any()) {

        // extract a group of threads with the same next PC among the active threads in the warp
        address_type tmp_next_pc = null_pc;
        simt_mask_t tmp_active_mask;
        for (int i = m_warp_size - 1; i >= 0; i--) {
            if ( top_active_mask.test(i) ) { // is this thread active?
                if (thread_done.test(i)) {
                    top_active_mask.reset(i); // remove completed thread from active mask
                } else if (tmp_next_pc == null_pc) {
                    tmp_next_pc = next_pc[i];
                    tmp_active_mask.set(i);
                    top_active_mask.reset(i);
                } else if (tmp_next_pc == next_pc[i]) {
                    tmp_active_mask.set(i);
                    top_active_mask.reset(i);
                }
            }
        }

        if(tmp_next_pc == null_pc) {
            assert(!top_active_mask.any()); // all threads done
            continue;
        }

        divergent_paths[tmp_next_pc]=tmp_active_mask;
        num_divergent_paths++;
    }
    if((next_inst_op == RET_OPS) && m_simt_splits_table->valid() && (num_divergent_paths==0)){
    	assert(m_simt_splits_table->valid());
    	m_simt_splits_table->invalidate();
    	m_simt_splits_table->update_active_entry();
       	bool warp_reaches_barrier = is_blocked();
        if(warp_reaches_barrier){
        	int cta_id = m_shader->get_cta_id(m_warp_id);
            m_shader->warp_reaches_barrier(cta_id,m_warp_id);
            if(m_shader->warp_count_at_barrier(cta_id)==0){
            	unsigned n = m_config->n_thread_per_shader / m_config->warp_size;
            	for(unsigned i=0; i<n; i++){
            		if(m_shader->get_cta_id(i)==cta_id)
            			m_shader->release_splits_barrier(i);
            	}
            }
        }
    	return;
    }
    address_type not_taken_pc = next_inst_pc+next_inst_size;
    assert(num_divergent_paths<=2);
    for(unsigned i=0; i<num_divergent_paths; i++){
    	address_type tmp_next_pc = null_pc;
    	simt_mask_t tmp_active_mask;
    	tmp_active_mask.reset();
    	/* Process the Not Taken first then the Taken Path */
    	if(divergent_paths.find(not_taken_pc)!=divergent_paths.end()){
    		assert(i==0);
    		tmp_next_pc=not_taken_pc;
    		tmp_active_mask=divergent_paths[tmp_next_pc];
    		divergent_paths.erase(tmp_next_pc);
    	}else{
    		std::map<address_type,simt_mask_t>:: iterator it=divergent_paths.begin();
    		tmp_next_pc=it->first;
    		tmp_active_mask=divergent_paths[tmp_next_pc];
    		divergent_paths.erase(tmp_next_pc);
    	}
    	assert(next_inst_op != RET_OPS);
        // HANDLE THE SPECIAL CASES FIRST
    	if (next_inst_op== CALL_OPS){
    		// Since call is not a divergent instruction, all threads should have executed a call instruction
    		assert(num_divergent_paths == 1);

    		simt_splits_table_entry new_st_entry;
    		new_st_entry.m_pc = tmp_next_pc;
    		new_st_entry.m_active_mask = tmp_active_mask;
    		new_st_entry.m_branch_div_cycle = gpu_sim_cycle+gpu_tot_sim_cycle;
    		new_st_entry.m_type = SPLITS_TABLE_TYPE_CALL;
	    	m_simt_splits_table->insert_new_entry(new_st_entry);
    		return;
    	}else if(next_inst_op == RET_OPS && top_type==SPLITS_TABLE_TYPE_CALL){
    		// pop the CALL Entry
    		assert(num_divergent_paths == 1);
    		m_simt_splits_table->invalidate();
        	m_simt_splits_table->update_active_entry();
    		simt_splits_table_entry new_st_entry;
    		new_st_entry.m_pc = tmp_next_pc;
    		new_st_entry.m_recvg_pc = top_recvg_pc;
    		new_st_entry.m_recvg_entry = top_recvg_entry;
    		new_st_entry.m_active_mask = tmp_active_mask;
    		new_st_entry.m_branch_div_cycle = gpu_sim_cycle+gpu_tot_sim_cycle;
    		new_st_entry.m_type = SPLITS_TABLE_TYPE_CALL;
	    	m_simt_splits_table->insert_new_entry(new_st_entry);
	    	return;
    	}else if(next_inst_op == RET_OPS){
    		abort();
    	}

		if (tmp_next_pc == top_recvg_pc){
    		//Direct reconvergence at top_recvg_pc: update Pending Mask in The RT table
			bool suspended = false;
			bool reconverged = m_simt_recvg_table->update_pending_mask(top_recvg_entry,top_recvg_pc,tmp_active_mask,suspended);
			if(reconverged && !suspended){
    			simt_mask_t  active_mask =  m_simt_recvg_table->get_active_mask(top_recvg_entry);
    			address_type pc = m_simt_recvg_table->get_pc(top_recvg_entry);
    	    	address_type rpc = m_simt_recvg_table->get_rpc(top_recvg_entry);
    	    	unsigned rpc_entry = m_simt_recvg_table->get_rpc_entry(top_recvg_entry);
    	        splits_table_entry_type type = m_simt_recvg_table->get_type(top_recvg_entry);
    	    	m_simt_splits_table->insert_new_entry(pc,rpc,rpc_entry,active_mask,type,true);
    		}
    		if(num_divergent_paths==1){
    			if(!invalidate){
    				m_simt_splits_table->invalidate();
        			invalidate=true;
    			}
    		}
    		continue;
    	}



        // this new entry is not converging
        // if this entry does not include thread from the warp, divergence occurs
    	if ((num_divergent_paths>1) && !warp_diverged ) {
            warp_diverged = true;
            // modify the existing top entry into a reconvergence entry in the RT table
            new_recvg_pc = recvg_pc;
            if (new_recvg_pc != top_recvg_pc) {
                // add a new reconvergence entry in the RT table
    	    	assert(top_recvg_pc>new_recvg_pc);
            	new_recvg_entry=m_simt_recvg_table->insert_new_entry(new_recvg_pc,top_recvg_pc,top_recvg_entry,top_active_mask_keep,SPLITS_TABLE_ENTRY_TYPE_NORMAL);
            }
        }

        // discard the new entry if its PC matches with reconvergence PC
        if (warp_diverged && tmp_next_pc == new_recvg_pc){
    		//Direct reconvergence at new_recvg_pc: update Pending Mask in The RT table
        	bool suspended = false;
    		bool reconverged = m_simt_recvg_table->update_pending_mask(new_recvg_entry,new_recvg_pc,tmp_active_mask,suspended);
    		if(reconverged && !suspended){
    			simt_mask_t  active_mask =  m_simt_recvg_table->get_active_mask(new_recvg_entry);
    			address_type pc = m_simt_recvg_table->get_pc(new_recvg_entry);
    	    	address_type rpc = m_simt_recvg_table->get_rpc(new_recvg_entry);
    	    	unsigned rpc_entry = m_simt_recvg_table->get_rpc_entry(new_recvg_entry);
    	    	splits_table_entry_type type = m_simt_recvg_table->get_type(new_recvg_entry);
    	    	m_simt_splits_table->insert_new_entry(pc,rpc,rpc_entry,active_mask,type);
    		}
    		continue;

        }

        // update the ST table
        if (warp_diverged) {
        	if(!invalidate){
        		m_simt_splits_table->invalidate();
        		invalidate = true;
        	}
        	m_simt_splits_table->insert_new_entry(tmp_next_pc,new_recvg_pc,new_recvg_entry,tmp_active_mask,SPLITS_TABLE_ENTRY_TYPE_NORMAL);
        }else{
        	m_simt_splits_table->update_pc(tmp_next_pc);
        	if(tmp_next_pc!=not_taken_pc){
        		m_simt_splits_table->push_back(); //even for .uni branches fifo would switch execution to another split
        	}
        }
    }

    if(invalidate){
    	m_simt_splits_table->update_active_entry();
    }


    check_simt_tables();


    if (warp_diverged) {
    	ptx_file_line_stats_add_warp_divergence(top_pc, 1);
    }
}
const simt_mask_t &simt_tables::get_active_mask()
{
	return m_simt_splits_table->get_active_mask();
}
void simt_tables::get_pdom_active_split_info( unsigned *pc, unsigned *rpc )
{
	m_simt_splits_table->get_pdom_active_split_info(pc, rpc);
}
unsigned simt_tables::get_rp()
{
	return m_simt_splits_table->get_rpc();
}
void     simt_tables::print(FILE*fp)
{
	printf("w%02d\n", m_warp_id);
	printf("Splits Table:\n");
	m_simt_splits_table->print(fp);
	printf("Reconvergence Table:\n");
	m_simt_recvg_table->print(fp);
}




simt_stack::simt_stack( unsigned wid, unsigned warpSize)
{
    m_warp_id=wid;
    m_warp_size = warpSize;
    reset();
}

void simt_stack::reset()
{
    m_stack.clear();
}

void simt_stack::launch( address_type start_pc, const simt_mask_t &active_mask )
{
    reset();
    simt_stack_entry new_stack_entry;
    new_stack_entry.m_pc = start_pc;
    new_stack_entry.m_calldepth = 1;
    new_stack_entry.m_active_mask = active_mask;
    new_stack_entry.m_type = STACK_ENTRY_TYPE_NORMAL;
    m_stack.push_back(new_stack_entry);
}

const simt_mask_t &simt_stack::get_active_mask() const
{
    assert(m_stack.size() > 0);
    return m_stack.back().m_active_mask;
}

void simt_stack::get_pdom_stack_top_info( unsigned *pc, unsigned *rpc ) const
{
   assert(m_stack.size() > 0);
   *pc = m_stack.back().m_pc;
   *rpc = m_stack.back().m_recvg_pc;
}

unsigned simt_stack::get_rp() const 
{ 
    assert(m_stack.size() > 0);
    return m_stack.back().m_recvg_pc;
}

void simt_stack::print (FILE *fout) const
{
    for ( unsigned k=0; k < m_stack.size(); k++ ) {
        simt_stack_entry stack_entry = m_stack[k];
        if ( k==0 ) {
            fprintf(fout, "w%02d %1u ", m_warp_id, k );
        } else {
            fprintf(fout, "    %1u ", k );
        }
        for (unsigned j=0; j<m_warp_size; j++)
            fprintf(fout, "%c", (stack_entry.m_active_mask.test(j)?'1':'0') );
        fprintf(fout, " pc: 0x%03x", stack_entry.m_pc );
        if ( stack_entry.m_recvg_pc == (unsigned)-1 ) {
            fprintf(fout," rp: ---- tp: %s cd: %2u ", (stack_entry.m_type==STACK_ENTRY_TYPE_CALL?"C":"N"), stack_entry.m_calldepth );
        } else {
            fprintf(fout," rp: %4u tp: %s cd: %2u ", stack_entry.m_recvg_pc, (stack_entry.m_type==STACK_ENTRY_TYPE_CALL?"C":"N"), stack_entry.m_calldepth );
        }
        if ( stack_entry.m_branch_div_cycle != 0 ) {
            fprintf(fout," bd@%6u ", (unsigned) stack_entry.m_branch_div_cycle );
        } else {
            fprintf(fout," " );
        }
        ptx_print_insn( stack_entry.m_pc, fout );
        fprintf(fout,"\n");
    }
}

void simt_stack::update( simt_mask_t &thread_done, addr_vector_t &next_pc, address_type recvg_pc, op_type next_inst_op,unsigned next_inst_size, address_type next_inst_pc )
{
    assert(m_stack.size() > 0);

    assert( next_pc.size() == m_warp_size );

    simt_mask_t  top_active_mask = m_stack.back().m_active_mask;
    address_type top_recvg_pc = m_stack.back().m_recvg_pc;
    address_type top_pc = m_stack.back().m_pc; // the pc of the instruction just executed
    stack_entry_type top_type = m_stack.back().m_type;
    assert(top_pc==next_inst_pc);
    assert(top_active_mask.any());

    const address_type null_pc = -1;
    bool warp_diverged = false;
    address_type new_recvg_pc = null_pc;
    unsigned num_divergent_paths=0;

    std::map<address_type,simt_mask_t> divergent_paths;
    while (top_active_mask.any()) {

        // extract a group of threads with the same next PC among the active threads in the warp
        address_type tmp_next_pc = null_pc;
        simt_mask_t tmp_active_mask;
        for (int i = m_warp_size - 1; i >= 0; i--) {
            if ( top_active_mask.test(i) ) { // is this thread active?
                if (thread_done.test(i)) {
                    top_active_mask.reset(i); // remove completed thread from active mask
                } else if (tmp_next_pc == null_pc) {
                    tmp_next_pc = next_pc[i];
                    tmp_active_mask.set(i);
                    top_active_mask.reset(i);
                } else if (tmp_next_pc == next_pc[i]) {
                    tmp_active_mask.set(i);
                    top_active_mask.reset(i);
                }
            }
        }

        if(tmp_next_pc == null_pc) {
            assert(!top_active_mask.any()); // all threads done
            continue;
        }

        divergent_paths[tmp_next_pc]=tmp_active_mask;
        num_divergent_paths++;
    }


    address_type not_taken_pc = next_inst_pc+next_inst_size;
    assert(num_divergent_paths<=2);
    for(unsigned i=0; i<num_divergent_paths; i++){
    	address_type tmp_next_pc = null_pc;
    	simt_mask_t tmp_active_mask;
    	tmp_active_mask.reset();
    	if(divergent_paths.find(not_taken_pc)!=divergent_paths.end()){
    		assert(i==0);
    		tmp_next_pc=not_taken_pc;
    		tmp_active_mask=divergent_paths[tmp_next_pc];
    		divergent_paths.erase(tmp_next_pc);
    	}else{
    		std::map<address_type,simt_mask_t>:: iterator it=divergent_paths.begin();
    		tmp_next_pc=it->first;
    		tmp_active_mask=divergent_paths[tmp_next_pc];
    		divergent_paths.erase(tmp_next_pc);
    	}

        // HANDLE THE SPECIAL CASES FIRST
    	if (next_inst_op== CALL_OPS){
    		// Since call is not a divergent instruction, all threads should have executed a call instruction
    		assert(num_divergent_paths == 1);

    		simt_stack_entry new_stack_entry;
    		new_stack_entry.m_pc = tmp_next_pc;
    		new_stack_entry.m_active_mask = tmp_active_mask;
    		new_stack_entry.m_branch_div_cycle = gpu_sim_cycle+gpu_tot_sim_cycle;
    		new_stack_entry.m_type = STACK_ENTRY_TYPE_CALL;
    		m_stack.push_back(new_stack_entry);
    		return;
    	}else if(next_inst_op == RET_OPS && top_type==STACK_ENTRY_TYPE_CALL){
    		// pop the CALL Entry
    		assert(num_divergent_paths == 1);
    		m_stack.pop_back();

    		assert(m_stack.size() > 0);
    		m_stack.back().m_pc=tmp_next_pc;// set the PC of the stack top entry to return PC from  the call stack;
            // Check if the New top of the stack is reconverging
            if (tmp_next_pc == m_stack.back().m_recvg_pc && m_stack.back().m_type!=STACK_ENTRY_TYPE_CALL){
            	assert(m_stack.back().m_type==STACK_ENTRY_TYPE_NORMAL);
            	m_stack.pop_back();
            }
            return;
    	}

        // discard the new entry if its PC matches with reconvergence PC
        // that automatically reconverges the entry
        // If the top stack entry is CALL, dont reconverge.
        if (tmp_next_pc == top_recvg_pc && (top_type != STACK_ENTRY_TYPE_CALL)) continue;

        // this new entry is not converging
        // if this entry does not include thread from the warp, divergence occurs
        if ((num_divergent_paths>1) && !warp_diverged ) {
            warp_diverged = true;
            // modify the existing top entry into a reconvergence entry in the pdom stack
            new_recvg_pc = recvg_pc;
            if (new_recvg_pc != top_recvg_pc) {
                m_stack.back().m_pc = new_recvg_pc;
                m_stack.back().m_branch_div_cycle = gpu_sim_cycle+gpu_tot_sim_cycle;

                m_stack.push_back(simt_stack_entry());
            }
        }

        // discard the new entry if its PC matches with reconvergence PC
        if (warp_diverged && tmp_next_pc == new_recvg_pc) continue;

        // update the current top of pdom stack
        m_stack.back().m_pc = tmp_next_pc;
        m_stack.back().m_active_mask = tmp_active_mask;
        if (warp_diverged) {
            m_stack.back().m_calldepth = 0;
            m_stack.back().m_recvg_pc = new_recvg_pc;
        } else {
            m_stack.back().m_recvg_pc = top_recvg_pc;
        }

        m_stack.push_back(simt_stack_entry());
    }
    assert(m_stack.size() > 0);
    m_stack.pop_back();


    if (warp_diverged) {
        ptx_file_line_stats_add_warp_divergence(top_pc, 1); 
    }
}

void core_t::execute_warp_inst_t(warp_inst_t &inst, unsigned warpId)
{
    for ( unsigned t=0; t < m_warp_size; t++ ) {
        if( inst.active(t) ) {
            if(warpId==(unsigned (-1)))
                warpId = inst.warp_id();
            unsigned tid=m_warp_size*warpId+t;
            m_thread[tid]->ptx_exec_inst(inst,t);
            
            //virtual function
            checkExecutionStatusAndUpdate(inst,t,tid);
        }
    } 
}
  
bool  core_t::ptx_thread_done( unsigned hw_thread_id ) const  
{
    return ((m_thread[ hw_thread_id ]==NULL) || m_thread[ hw_thread_id ]->is_done());
}


void core_t::updateSIMTDivergenceStructures(unsigned warpId, warp_inst_t * inst)
{
    simt_mask_t thread_done;
    addr_vector_t next_pc;
    unsigned wtid = warpId * m_warp_size;
    for (unsigned i = 0; i < m_warp_size; i++) {
        if( ptx_thread_done(wtid+i) ) {
            thread_done.set(i);
            next_pc.push_back( (address_type)-1 );
        } else {
            if( inst->reconvergence_pc == RECONVERGE_RETURN_PC ) 
                inst->reconvergence_pc = get_return_pc(m_thread[wtid+i]);
            next_pc.push_back( m_thread[wtid+i]->get_pc() );
        }
    }
    if( m_gpu->simd_model() == POST_DOMINATOR ) {
    	m_simt_stack[warpId]->update(thread_done,next_pc,inst->reconvergence_pc, inst->op,inst->isize,inst->pc);
    }else{
    	m_simt_tables[warpId]->update(thread_done,next_pc,inst->reconvergence_pc, inst->op,inst->isize,inst->pc);
    }
}

unsigned core_t::getInsertionDist(unsigned wid)
{
	return m_simt_tables[wid]->getInsertionDist();
}

unsigned long long core_t::getInsertionCycle(unsigned wid)
{
	return m_simt_tables[wid]->getInsertionCycle();
}

unsigned core_t::getSTSize(unsigned wid)
{
	return m_simt_tables[wid]->getSTsize();
}

unsigned core_t::getRTSize(unsigned wid)
{
	return m_simt_tables[wid]->getRTsize();
}


//! Get the warp to be executed using the data taken form the SIMT stack
warp_inst_t core_t::getExecuteWarp(unsigned warpId)
{
    unsigned pc,rpc;
    if( m_gpu->simd_model() == POST_DOMINATOR ) {
        m_simt_stack[warpId]->get_pdom_stack_top_info(&pc,&rpc);
    }else{
        m_simt_tables[warpId]->get_pdom_active_split_info(&pc,&rpc);
    }
    warp_inst_t wi= *ptx_fetch_inst(pc);
    if( m_gpu->simd_model() == POST_DOMINATOR ) {
        wi.set_active(m_simt_stack[warpId]->get_active_mask());
    }else{
        wi.set_active(m_simt_tables[warpId]->get_active_mask());
    }
    return wi;
}

void core_t::deleteSIMTDivergenceStructures()
{
    if( m_gpu->simd_model() == POST_DOMINATOR ) {
        if ( m_simt_stack ) {
            for (unsigned i = 0; i < m_warp_count; ++i)
                delete m_simt_stack[i];
            delete[] m_simt_stack;
            m_simt_stack = NULL;
        }
    }else{
    	if ( m_simt_tables ) {
    		for (unsigned i = 0; i < m_warp_count; ++i)
    			delete m_simt_tables[i];
    		delete[] m_simt_tables;
    		m_simt_tables = NULL;
    	}
    }
}




void core_t::initilizeSIMTDivergenceStructures(unsigned warp_count, unsigned warp_size)
{ 
    if( m_gpu->simd_model() == POST_DOMINATOR ) {
    	m_simt_stack = new simt_stack*[warp_count];
    	for (unsigned i = 0; i < warp_count; ++i)
    		m_simt_stack[i] = new simt_stack(i,warp_size);
    	m_warp_size = warp_size;
    	m_warp_count = warp_count;
    }else{
        m_simt_tables = new simt_tables*[warp_count];
        for (unsigned i = 0; i < warp_count; ++i)
            m_simt_tables[i] = new simt_tables(i,warp_size,m_gpu->get_shader_config(),m_gpu->getMemoryConfig());
        m_warp_size = warp_size;
        m_warp_count = warp_count;
    }
}


void core_t::get_pdom_stack_top_info( unsigned warpId, unsigned *pc, unsigned *rpc ) const
{
    if( m_gpu->simd_model() == POST_DOMINATOR ) {
        m_simt_stack[warpId]->get_pdom_stack_top_info(pc,rpc);
    }else{
        m_simt_tables[warpId]->get_pdom_active_split_info(pc,rpc);
    }

}
