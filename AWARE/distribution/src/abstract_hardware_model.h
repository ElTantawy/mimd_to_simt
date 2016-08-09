// Copyright (c) 2009-2011, Tor M. Aamodt, Inderpreet Singh, Ahmed ElTantawy
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

#ifndef ABSTRACT_HARDWARE_MODEL_INCLUDED
#define ABSTRACT_HARDWARE_MODEL_INCLUDED

// Forward declarations
class gpgpu_sim;
class kernel_info_t;
class shader_core_ctx;
struct shader_core_config;
class simt_tables;

//Set a hard limit of 32 CTAs per shader [cuda only has 8]
#define MAX_CTA_PER_SHADER 32
#define MAX_BARRIERS_PER_CTA 16



enum _memory_space_t {
   undefined_space=0,
   reg_space,
   local_space,
   shared_space,
   param_space_unclassified,
   param_space_kernel,  /* global to all threads in a kernel : read-only */
   param_space_local,   /* local to a thread : read-writable */
   const_space,
   tex_space,
   surf_space,
   global_space,
   generic_space,
   instruction_space
};


enum FuncCache
{
  FuncCachePreferNone = 0,
  FuncCachePreferShared = 1,
  FuncCachePreferL1 = 2
};


#ifdef __cplusplus

#include <string.h>
#include <stdio.h>
#include <queue>

typedef unsigned long long new_addr_type;
typedef unsigned address_type;
typedef unsigned addr_t;

// the following are operations the timing model can see 

enum uarch_op_t {
   NO_OP=-1,
   ALU_OP=1,
   SFU_OP,
   ALU_SFU_OP,
   LOAD_OP,
   STORE_OP,
   BRANCH_OP,
   BARRIER_OP,
   MEMORY_BARRIER_OP,
   CALL_OPS,
   RET_OPS
};
typedef enum uarch_op_t op_type;


enum uarch_bar_t {
   NOT_BAR=-1,
   SYNC=1,
   ARRIVE,
   RED
};
typedef enum uarch_bar_t barrier_type;

enum uarch_red_t {
   NOT_RED=-1,
   POPC_RED=1,
   AND_RED,
   OR_RED
};
typedef enum uarch_red_t reduction_type;


enum uarch_operand_type_t {
	UN_OP=-1,
    INT_OP,
    FP_OP
};
typedef enum uarch_operand_type_t types_of_operands;

enum special_operations_t {
    OTHER_OP,
    INT__OP,
	INT_MUL24_OP,
	INT_MUL32_OP,
	INT_MUL_OP,
    INT_DIV_OP,
    FP_MUL_OP,
    FP_DIV_OP,
    FP__OP,
	FP_SQRT_OP,
	FP_LG_OP,
	FP_SIN_OP,
	FP_EXP_OP
};
typedef enum special_operations_t special_ops; // Required to identify for the power model
enum operation_pipeline_t {
    UNKOWN_OP,
    SP__OP,
    SFU__OP,
    MEM__OP
};
typedef enum operation_pipeline_t operation_pipeline;
enum mem_operation_t {
    NOT_TEX,
    TEX
};
typedef enum mem_operation_t mem_operation;

enum _memory_op_t {
	no_memory_op = 0,
	bru_st_fill_request,
	bru_st_spill_request,
	bru_rt_fill_request,
	bru_rt_spill_request,
	memory_load,
	memory_store
};

#include <bitset>
#include <list>
#include <vector>
#include <assert.h>
#include <stdlib.h>
#include <map>
#include <deque>
#include <stack>

#if !defined(__VECTOR_TYPES_H__)
struct dim3 {
   unsigned int x, y, z;
};
#endif

void increment_x_then_y_then_z( dim3 &i, const dim3 &bound);

class kernel_info_t {
public:
//   kernel_info_t()
//   {
//      m_valid=false;
//      m_kernel_entry=NULL;
//      m_uid=0;
//      m_num_cores_running=0;
//      m_param_mem=NULL;
//   }
   kernel_info_t( dim3 gridDim, dim3 blockDim, class function_info *entry );
   ~kernel_info_t();

   void inc_running() { m_num_cores_running++; }
   void dec_running()
   {
       assert( m_num_cores_running > 0 );
       m_num_cores_running--; 
   }
   bool running() const { return m_num_cores_running>0; }
   bool done() const 
   {
       return no_more_ctas_to_run() && !running();
   }
   class function_info *entry() { return m_kernel_entry; }
   const class function_info *entry() const { return m_kernel_entry; }

   size_t num_blocks() const
   {
      return m_grid_dim.x * m_grid_dim.y * m_grid_dim.z;
   }

   size_t threads_per_cta() const
   {
      return m_block_dim.x * m_block_dim.y * m_block_dim.z;
   } 

   dim3 get_grid_dim() const { return m_grid_dim; }
   dim3 get_cta_dim() const { return m_block_dim; }

   void increment_cta_id() 
   { 
      increment_x_then_y_then_z(m_next_cta,m_grid_dim); 
      m_next_tid.x=0;
      m_next_tid.y=0;
      m_next_tid.z=0;
   }
   dim3 get_next_cta_id() const { return m_next_cta; }
   bool no_more_ctas_to_run() const 
   {
      return (m_next_cta.x >= m_grid_dim.x || m_next_cta.y >= m_grid_dim.y || m_next_cta.z >= m_grid_dim.z );
   }

   void increment_thread_id() { increment_x_then_y_then_z(m_next_tid,m_block_dim); }
   dim3 get_next_thread_id_3d() const  { return m_next_tid; }
   unsigned get_next_thread_id() const 
   { 
      return m_next_tid.x + m_block_dim.x*m_next_tid.y + m_block_dim.x*m_block_dim.y*m_next_tid.z;
   }
   bool more_threads_in_cta() const 
   {
      return m_next_tid.z < m_block_dim.z && m_next_tid.y < m_block_dim.y && m_next_tid.x < m_block_dim.x;
   }
   unsigned get_uid() const { return m_uid; }
   std::string name() const;

   std::list<class ptx_thread_info *> &active_threads() { return m_active_threads; }
   class memory_space *get_param_memory() { return m_param_mem; }

private:
   kernel_info_t( const kernel_info_t & ); // disable copy constructor
   void operator=( const kernel_info_t & ); // disable copy operator

   class function_info *m_kernel_entry;

   unsigned m_uid;
   static unsigned m_next_uid;

   dim3 m_grid_dim;
   dim3 m_block_dim;
   dim3 m_next_cta;
   dim3 m_next_tid;

   unsigned m_num_cores_running;

   std::list<class ptx_thread_info *> m_active_threads;
   class memory_space *m_param_mem;
};

struct core_config {
    core_config() 
    { 
        m_valid = false; 
        num_shmem_bank=16; 
        shmem_limited_broadcast = false; 
        gpgpu_shmem_sizeDefault=(unsigned)-1;
        gpgpu_shmem_sizePrefL1=(unsigned)-1;
        gpgpu_shmem_sizePrefShared=(unsigned)-1;
    }
    virtual void init() = 0;

    bool m_valid;
    unsigned warp_size;

    // off-chip memory request architecture parameters
    int gpgpu_coalesce_arch;

    // shared memory bank conflict checking parameters
    bool shmem_limited_broadcast;
    static const address_type WORD_SIZE=4;
    unsigned num_shmem_bank;
    unsigned shmem_bank_func(address_type addr) const
    {
        return ((addr/WORD_SIZE) % num_shmem_bank);
    }
    unsigned mem_warp_parts;  
    mutable unsigned gpgpu_shmem_size;
    unsigned gpgpu_shmem_sizeDefault;
    unsigned gpgpu_shmem_sizePrefL1;
    unsigned gpgpu_shmem_sizePrefShared;

    // texture and constant cache line sizes (used to determine number of memory accesses)
    unsigned gpgpu_cache_texl1_linesize;
    unsigned gpgpu_cache_constl1_linesize;

	unsigned gpgpu_max_insn_issue_per_warp;
};

// bounded stack that implements simt reconvergence using pdom mechanism from MICRO'07 paper
const unsigned MAX_WARP_SIZE = 32;
typedef std::bitset<MAX_WARP_SIZE> active_mask_t;
#define MAX_WARP_SIZE_SIMT_STACK  MAX_WARP_SIZE
typedef std::bitset<MAX_WARP_SIZE_SIMT_STACK> simt_mask_t;
typedef std::vector<address_type> addr_vector_t;


#define GLOBAL_HEAP_START 0x80000000
   // start allocating from this address (lower values used for allocating globals in .ptx file)
#define SHARED_MEM_SIZE_MAX (64*1024)
#define LOCAL_MEM_SIZE_MAX (8*1024)
#define MAX_STREAMING_MULTIPROCESSORS 64
#define MAX_THREAD_PER_SM 2048
#define MAX_BRU_VIR_PER_SPLIT (16*2)
#define TOTAL_BRU_VIR (MAX_STREAMING_MULTIPROCESSORS*MAX_THREAD_PER_SM*MAX_BRU_VIR_PER_SPLIT)
#define TOTAL_LOCAL_MEM_PER_SM (MAX_THREAD_PER_SM*LOCAL_MEM_SIZE_MAX)
#define TOTAL_SHARED_MEM (MAX_STREAMING_MULTIPROCESSORS*SHARED_MEM_SIZE_MAX)
#define TOTAL_LOCAL_MEM (MAX_STREAMING_MULTIPROCESSORS*MAX_THREAD_PER_SM*LOCAL_MEM_SIZE_MAX)
#define BRU_VIR_START (GLOBAL_HEAP_START-TOTAL_BRU_VIR)
#define SHARED_GENERIC_START (BRU_VIR_START-TOTAL_SHARED_MEM)
#define LOCAL_GENERIC_START (SHARED_GENERIC_START-TOTAL_LOCAL_MEM)
#define STATIC_ALLOC_LIMIT (GLOBAL_HEAP_START - (TOTAL_LOCAL_MEM+TOTAL_SHARED_MEM+TOTAL_BRU_VIR))

#if !defined(__CUDA_RUNTIME_API_H__)

enum cudaChannelFormatKind {
   cudaChannelFormatKindSigned,
   cudaChannelFormatKindUnsigned,
   cudaChannelFormatKindFloat
};

struct cudaChannelFormatDesc {
   int                        x;
   int                        y;
   int                        z;
   int                        w;
   enum cudaChannelFormatKind f;
};

struct cudaArray {
   void *devPtr;
   int devPtr32;
   struct cudaChannelFormatDesc desc;
   int width;
   int height;
   int size; //in bytes
   unsigned dimensions;
};

enum cudaTextureAddressMode {
   cudaAddressModeWrap,
   cudaAddressModeClamp
};

enum cudaTextureFilterMode {
   cudaFilterModePoint,
   cudaFilterModeLinear
};

enum cudaTextureReadMode {
   cudaReadModeElementType,
   cudaReadModeNormalizedFloat
};

struct textureReference {
   int                           normalized;
   enum cudaTextureFilterMode    filterMode;
   enum cudaTextureAddressMode   addressMode[3];
   struct cudaChannelFormatDesc  channelDesc;
};

#endif

// Struct that record other attributes in the textureReference declaration 
// - These attributes are passed thru __cudaRegisterTexture()
struct textureReferenceAttr {
    const struct textureReference *m_texref; 
    int m_dim; 
    enum cudaTextureReadMode m_readmode; 
    int m_ext; 
    textureReferenceAttr(const struct textureReference *texref, 
                         int dim, 
                         enum cudaTextureReadMode readmode, 
                         int ext)
    : m_texref(texref), m_dim(dim), m_readmode(readmode), m_ext(ext) 
    {  }
};

class gpgpu_functional_sim_config 
{
public:
    void reg_options(class OptionParser * opp);

    void ptx_set_tex_cache_linesize(unsigned linesize);

    unsigned get_forced_max_capability() const { return m_ptx_force_max_capability; }
    bool convert_to_ptxplus() const { return m_ptx_convert_to_ptxplus; }
    bool use_cuobjdump() const { return m_ptx_use_cuobjdump; }
    bool experimental_lib_support() const { return m_experimental_lib_support; }

    int         get_ptx_inst_debug_to_file() const { return g_ptx_inst_debug_to_file; }
    const char* get_ptx_inst_debug_file() const  { return g_ptx_inst_debug_file; }
    int         get_ptx_inst_debug_thread_uid() const { return g_ptx_inst_debug_thread_uid; }
    unsigned    get_texcache_linesize() const { return m_texcache_linesize; }

private:
    // PTX options
    int m_ptx_convert_to_ptxplus;
    int m_ptx_use_cuobjdump;
    int m_experimental_lib_support;
    unsigned m_ptx_force_max_capability;

    int   g_ptx_inst_debug_to_file;
    char* g_ptx_inst_debug_file;
    int   g_ptx_inst_debug_thread_uid;

    unsigned m_texcache_linesize;
};

class gpgpu_t {
public:
    gpgpu_t( const gpgpu_functional_sim_config &config );
    void* gpu_malloc( size_t size );
    void* gpu_mallocarray( size_t count );
    void  gpu_memset( size_t dst_start_addr, int c, size_t count );
    void  memcpy_to_gpu( size_t dst_start_addr, const void *src, size_t count );
    void  memcpy_from_gpu( void *dst, size_t src_start_addr, size_t count );
    void  memcpy_gpu_to_gpu( size_t dst, size_t src, size_t count );
    
    class memory_space *get_global_memory() { return m_global_mem; }
    class memory_space *get_tex_memory() { return m_tex_mem; }
    class memory_space *get_surf_memory() { return m_surf_mem; }

    void gpgpu_ptx_sim_bindTextureToArray(const struct textureReference* texref, const struct cudaArray* array);
    void gpgpu_ptx_sim_bindNameToTexture(const char* name, const struct textureReference* texref, int dim, int readmode, int ext);
    const char* gpgpu_ptx_sim_findNamefromTexture(const struct textureReference* texref);

    const struct textureReference* get_texref(const std::string &texname) const
    {
        std::map<std::string, const struct textureReference*>::const_iterator t=m_NameToTextureRef.find(texname);
        assert( t != m_NameToTextureRef.end() );
        return t->second;
    }
    const struct cudaArray* get_texarray( const struct textureReference *texref ) const
    {
        std::map<const struct textureReference*,const struct cudaArray*>::const_iterator t=m_TextureRefToCudaArray.find(texref);
        assert(t != m_TextureRefToCudaArray.end());
        return t->second;
    }
    const struct textureInfo* get_texinfo( const struct textureReference *texref ) const
    {
        std::map<const struct textureReference*, const struct textureInfo*>::const_iterator t=m_TextureRefToTexureInfo.find(texref);
        assert(t != m_TextureRefToTexureInfo.end());
        return t->second;
    }

    const struct textureReferenceAttr* get_texattr( const struct textureReference *texref ) const
    {
        std::map<const struct textureReference*, const struct textureReferenceAttr*>::const_iterator t=m_TextureRefToAttribute.find(texref);
        assert(t != m_TextureRefToAttribute.end());
        return t->second;
    }

    const gpgpu_functional_sim_config &get_config() const { return m_function_model_config; }
    FILE* get_ptx_inst_debug_file() { return ptx_inst_debug_file; }

protected:
    const gpgpu_functional_sim_config &m_function_model_config;
    FILE* ptx_inst_debug_file;

    class memory_space *m_global_mem;
    class memory_space *m_tex_mem;
    class memory_space *m_surf_mem;
    
    unsigned long long m_dev_malloc;
    
    std::map<std::string, const struct textureReference*> m_NameToTextureRef;
    std::map<const struct textureReference*,const struct cudaArray*> m_TextureRefToCudaArray;
    std::map<const struct textureReference*, const struct textureInfo*> m_TextureRefToTexureInfo;
    std::map<const struct textureReference*, const struct textureReferenceAttr*> m_TextureRefToAttribute;
};

struct gpgpu_ptx_sim_kernel_info 
{
   // Holds properties of the kernel (Kernel's resource use). 
   // These will be set to zero if a ptxinfo file is not present.
   int lmem;
   int smem;
   int cmem;
   int regs;
   unsigned ptx_version;
   unsigned sm_target;
};

struct gpgpu_ptx_sim_arg {
   gpgpu_ptx_sim_arg() { m_start=NULL; }
   gpgpu_ptx_sim_arg(const void *arg, size_t size, size_t offset)
   {
      m_start=arg;
      m_nbytes=size;
      m_offset=offset;
   }
   const void *m_start;
   size_t m_nbytes;
   size_t m_offset;
};

typedef std::list<gpgpu_ptx_sim_arg> gpgpu_ptx_sim_arg_list_t;

class memory_space_t {
public:
   memory_space_t() { m_type = undefined_space; m_bank=0; }
   memory_space_t( const enum _memory_space_t &from ) { m_type = from; m_bank = 0; }
   bool operator==( const memory_space_t &x ) const { return (m_bank == x.m_bank) && (m_type == x.m_type); }
   bool operator!=( const memory_space_t &x ) const { return !(*this == x); }
   bool operator<( const memory_space_t &x ) const 
   { 
      if(m_type < x.m_type)
         return true;
      else if(m_type > x.m_type)
         return false;
      else if( m_bank < x.m_bank )
         return true; 
      return false;
   }
   enum _memory_space_t get_type() const { return m_type; }
   unsigned get_bank() const { return m_bank; }
   void set_bank( unsigned b ) { m_bank = b; }
   bool is_const() const { return (m_type == const_space) || (m_type == param_space_kernel); }
   bool is_local() const { return (m_type == local_space) || (m_type == param_space_local); }
   bool is_global() const { return (m_type == global_space); }

private:
   enum _memory_space_t m_type;
   unsigned m_bank; // n in ".const[n]"; note .const == .const[0] (see PTX 2.1 manual, sec. 5.1.3)
};

const unsigned MAX_MEMORY_ACCESS_SIZE = 128;
typedef std::bitset<MAX_MEMORY_ACCESS_SIZE> mem_access_byte_mask_t;
#define NO_PARTIAL_WRITE (mem_access_byte_mask_t())

#define MEM_ACCESS_TYPE_TUP_DEF \
MA_TUP_BEGIN( mem_access_type ) \
   MA_TUP( GLOBAL_ACC_R ), \
   MA_TUP( LOCAL_ACC_R ), \
   MA_TUP( CONST_ACC_R ), \
   MA_TUP( TEXTURE_ACC_R ), \
   MA_TUP( GLOBAL_ACC_W ), \
   MA_TUP( BRU_ST_SPILL ), \
   MA_TUP( BRU_ST_FILL ), \
   MA_TUP( BRU_RT_SPILL ), \
   MA_TUP( BRU_RT_FILL ), \
   MA_TUP( LOCAL_ACC_W ), \
   MA_TUP( L1_WRBK_ACC ), \
   MA_TUP( L2_WRBK_ACC ), \
   MA_TUP( INST_ACC_R ), \
   MA_TUP( L1_WR_ALLOC_R ), \
   MA_TUP( L2_WR_ALLOC_R ), \
   MA_TUP( NUM_MEM_ACCESS_TYPE ) \
MA_TUP_END( mem_access_type ) 

#define MA_TUP_BEGIN(X) enum X {
#define MA_TUP(X) X
#define MA_TUP_END(X) };
MEM_ACCESS_TYPE_TUP_DEF
#undef MA_TUP_BEGIN
#undef MA_TUP
#undef MA_TUP_END

const char * mem_access_type_str(enum mem_access_type access_type); 

enum cache_operator_type {
    CACHE_UNDEFINED, 

    // loads
    CACHE_ALL,          // .ca
    CACHE_LAST_USE,     // .lu
    CACHE_VOLATILE,     // .cv
                       
    // loads and stores 
    CACHE_STREAMING,    // .cs
    CACHE_GLOBAL,       // .cg

    // stores
    CACHE_WRITE_BACK,   // .wb
    CACHE_WRITE_THROUGH // .wt
};

class mem_access_t {
public:
   mem_access_t() { init(); }
   mem_access_t( mem_access_type type, 
                 new_addr_type address, 
                 unsigned size,
                 bool wr )
   {
       init();
       m_type = type;
       m_addr = address;
       m_req_size = size;
       m_write = wr;
   }
   mem_access_t( mem_access_type type, 
                 new_addr_type address, 
                 unsigned size, 
                 bool wr, 
                 const active_mask_t &active_mask,
                 const mem_access_byte_mask_t &byte_mask )
    : m_warp_mask(active_mask), m_byte_mask(byte_mask)
   {
      init();
      m_type = type;
      m_addr = address;
      m_req_size = size;
      m_write = wr;
   }

   new_addr_type get_addr() const { return m_addr; }
   void set_addr(new_addr_type addr) {m_addr=addr;}
   unsigned get_size() const { return m_req_size; }
   const active_mask_t &get_warp_mask() const { return m_warp_mask; }
   bool is_write() const { return m_write; }
   enum mem_access_type get_type() const { return m_type; }
   mem_access_byte_mask_t get_byte_mask() const { return m_byte_mask; }

   void print(FILE *fp) const
   {
       fprintf(fp,"addr=0x%llx, %s, size=%u, ", m_addr, m_write?"store":"load ", m_req_size );
       switch(m_type) {
       case GLOBAL_ACC_R:  fprintf(fp,"GLOBAL_R"); break;
       case LOCAL_ACC_R:   fprintf(fp,"LOCAL_R "); break;
       case CONST_ACC_R:   fprintf(fp,"CONST   "); break;
       case TEXTURE_ACC_R: fprintf(fp,"TEXTURE "); break;
       case GLOBAL_ACC_W:  fprintf(fp,"GLOBAL_W"); break;
       case LOCAL_ACC_W:   fprintf(fp,"LOCAL_W "); break;
       case L2_WRBK_ACC:   fprintf(fp,"L2_WRBK "); break;
       case INST_ACC_R:    fprintf(fp,"INST    "); break;
       case L1_WRBK_ACC:   fprintf(fp,"L1_WRBK "); break;
       default:            fprintf(fp,"unknown "); break;
       }
   }

private:
   void init() 
   {
      m_uid=++sm_next_access_uid;
      m_addr=0;
      m_req_size=0;
   }

   unsigned      m_uid;
   new_addr_type m_addr;     // request address
   bool          m_write;
   unsigned      m_req_size; // bytes
   mem_access_type m_type;
   active_mask_t m_warp_mask;
   mem_access_byte_mask_t m_byte_mask;

   static unsigned sm_next_access_uid;
};

class mem_fetch;

class mem_fetch_interface {
public:
    virtual bool full( unsigned size, bool write ) const = 0;
    virtual void push( mem_fetch *mf ) = 0;
};

class mem_fetch_allocator {
public:
    virtual mem_fetch *alloc( new_addr_type addr, mem_access_type type, unsigned size, bool wr ) const = 0;
    virtual mem_fetch *alloc( const class warp_inst_t &inst, const mem_access_t &access ) const = 0;
};

// the maximum number of destination, source, or address uarch operands in a instruction
#define MAX_REG_OPERANDS 8

struct dram_callback_t {
   dram_callback_t() { function=NULL; instruction=NULL; thread=NULL; }
   void (*function)(const class inst_t*, class ptx_thread_info*);

   const class inst_t* instruction;
   class ptx_thread_info *thread;
};

class inst_t {
public:
    inst_t()
    {
        m_decoded=false;
        pc=(address_type)-1;
        reconvergence_pc=(address_type)-1;
        op=NO_OP;
        bar_type=NOT_BAR;
        red_type=NOT_RED;
        bar_id=(unsigned)-1;
        bar_count=(unsigned)-1;
        oprnd_type=UN_OP;
        sp_op=OTHER_OP;
        op_pipe=UNKOWN_OP;
        mem_op=NOT_TEX;
        num_operands=0;
        num_regs=0;
        memset(out, 0, sizeof(unsigned)); 
        memset(in, 0, sizeof(unsigned)); 
        is_vectorin=0; 
        is_vectorout=0;
        space = memory_space_t();
        cache_op = CACHE_UNDEFINED;
        latency = 1;
        initiation_interval = 1;
        for( unsigned i=0; i < MAX_REG_OPERANDS; i++ ) {
            arch_reg.src[i] = -1;
            arch_reg.dst[i] = -1;
        }
        isize=0;
    }
    bool valid() const { return m_decoded; }
    virtual void print_insn( FILE *fp ) const 
    {
        fprintf(fp," [inst @ pc=0x%04x] ", pc );
    }
    bool is_bru_st_fill_request() const { return (op == LOAD_OP && memory_op == bru_st_fill_request);}
    bool is_bru_rt_fill_request() const { return (op == LOAD_OP && memory_op == bru_rt_fill_request);}
    bool is_bru_st_spill_request() const { return (op == STORE_OP && memory_op == bru_st_spill_request);}
    bool is_bru_rt_spill_request() const { return (op == STORE_OP && memory_op == bru_rt_spill_request);}
    bool is_load() const { return ((op == LOAD_OP || memory_op == memory_load) && memory_op != bru_st_fill_request && memory_op != bru_rt_fill_request); }
    bool is_store() const { return ((op == STORE_OP || memory_op == memory_store) && memory_op != bru_st_spill_request && memory_op != bru_rt_spill_request); }
    unsigned get_num_operands() const {return num_operands;}
    unsigned get_num_regs() const {return num_regs;}
    void set_num_regs(unsigned num) {num_regs=num;}
    void set_num_operands(unsigned num) {num_operands=num;}
    void set_bar_id(unsigned id) {bar_id=id;}
    void set_bar_count(unsigned count) {bar_count=count;}

    address_type pc;        // program counter address of instruction
    unsigned isize;         // size of instruction in bytes 
    op_type op;             // opcode (uarch visible)

    barrier_type bar_type;
    reduction_type red_type;
    unsigned bar_id;
    unsigned bar_count;

    types_of_operands oprnd_type;     // code (uarch visible) identify if the operation is an interger or a floating point
    special_ops sp_op;           // code (uarch visible) identify if int_alu, fp_alu, int_mul ....
    operation_pipeline op_pipe;  // code (uarch visible) identify the pipeline of the operation (SP, SFU or MEM)
    mem_operation mem_op;        // code (uarch visible) identify memory type
    _memory_op_t memory_op; // memory_op used by ptxplus 
    unsigned num_operands;
    unsigned num_regs; // count vector operand as one register operand

    address_type reconvergence_pc; // -1 => not a branch, -2 => use function return address
    
    unsigned out[4];
    unsigned in[4];
    unsigned char is_vectorin;
    unsigned char is_vectorout;
    int pred; // predicate register number
    int ar1, ar2;
    // register number for bank conflict evaluation
    struct {
        int dst[MAX_REG_OPERANDS];
        int src[MAX_REG_OPERANDS];
    } arch_reg;
    //int arch_reg[MAX_REG_OPERANDS]; // register number for bank conflict evaluation
    unsigned latency; // operation latency 
    unsigned initiation_interval;

    unsigned data_size; // what is the size of the word being operated on?
    memory_space_t space;
    cache_operator_type cache_op;

protected:
    bool m_decoded;
    virtual void pre_decode() {}
};

enum splits_replacement_policy_t {
   FIFO_BACK = 1,
   NUM_ST_REPLACEMENT_POLICIES
};

enum reconvergence_replacement_policy_t {
   REC_LRU = 1,
   NUM_REC_REPLACEMENT_POLICIES
};

enum divergence_support_t {
   POST_DOMINATOR = 1,
   AWARE_RECONVERGENCE = 2,
   NUM_SIMD_MODEL
};

const unsigned MAX_ACCESSES_PER_INSN_PER_THREAD = 8;

class warp_inst_t: public inst_t {
public:
    // constructors
    warp_inst_t() 
    {
        m_uid=0;
        m_empty=true; 
        m_config=NULL; 
    }
    warp_inst_t( const core_config *config ) 
    { 
        m_uid=0;
        assert(config->warp_size<=MAX_WARP_SIZE); 
        m_config=config;
        m_empty=true; 
        m_isatomic=false;
        m_per_scalar_thread_valid=false;
        m_mem_accesses_created=false;
        m_cache_hit=false;
        m_is_printf=false;
    }
    virtual ~warp_inst_t(){
    }

    // modifiers
    void broadcast_barrier_reduction( const active_mask_t& access_mask);
    void do_atomic(bool forceDo=false);
    void do_atomic( const active_mask_t& access_mask, bool forceDo=false );
    void clear() 
    { 
        m_empty=true; 
    }
    void clear_pending_mem_requests()
    {
    	m_accessq.clear();
    }
    void issue( const active_mask_t &mask, unsigned warp_id, unsigned long long cycle, int dynamic_warp_id ) 
    {
        m_warp_active_mask = mask;
        m_warp_issued_mask = mask; 
        m_uid = ++sm_next_uid;
        m_warp_id = warp_id;
        m_dynamic_warp_id = dynamic_warp_id;
        issue_cycle = cycle;
        cycles = initiation_interval;
        m_cache_hit=false;
        m_empty=false;
    }
    const active_mask_t & get_active_mask() const
    {
    	return m_warp_active_mask;
    }
    void completed( unsigned long long cycle ) const;  // stat collection: called when the instruction is completed  

    void set_addr( unsigned n, new_addr_type addr ) 
    {
        if( !m_per_scalar_thread_valid ) {
            m_per_scalar_thread.resize(m_config->warp_size);
            m_per_scalar_thread_valid=true;
        }
        m_per_scalar_thread[n].memreqaddr[0] = addr;
    }
    void set_addr( unsigned n, new_addr_type* addr, unsigned num_addrs )
    {
        if( !m_per_scalar_thread_valid ) {
            m_per_scalar_thread.resize(m_config->warp_size);
            m_per_scalar_thread_valid=true;
        }
        assert(num_addrs <= MAX_ACCESSES_PER_INSN_PER_THREAD);
        for(unsigned i=0; i<num_addrs; i++)
            m_per_scalar_thread[n].memreqaddr[i] = addr[i];
    }

    struct transaction_info {
        std::bitset<4> chunks; // bitmask: 32-byte chunks accessed
        mem_access_byte_mask_t bytes;
        active_mask_t active; // threads in this transaction

        bool test_bytes(unsigned start_bit, unsigned end_bit) {
           for( unsigned i=start_bit; i<=end_bit; i++ )
              if(bytes.test(i))
                 return true;
           return false;
        }
    };

    void inject_mem_acccesses(mem_access_t acc);
    void generate_mem_accesses();
    void memory_coalescing_arch_13( bool is_write, mem_access_type access_type );
    void memory_coalescing_arch_13_atomic( bool is_write, mem_access_type access_type );
    void memory_coalescing_arch_13_reduce_and_send( bool is_write, mem_access_type access_type, const transaction_info &info, new_addr_type addr, unsigned segment_size );

    void add_callback( unsigned lane_id, 
                       void (*function)(const class inst_t*, class ptx_thread_info*),
                       const inst_t *inst, 
                       class ptx_thread_info *thread,
                       bool atomic)
    {
        if( !m_per_scalar_thread_valid ) {
            m_per_scalar_thread.resize(m_config->warp_size);
            m_per_scalar_thread_valid=true;
            if(atomic) m_isatomic=true;
        }
        m_per_scalar_thread[lane_id].callback.function = function;
        m_per_scalar_thread[lane_id].callback.instruction = inst;
        m_per_scalar_thread[lane_id].callback.thread = thread;
    }
    void set_active( const active_mask_t &active );

    void clear_active( const active_mask_t &inactive );
    void set_not_active( unsigned lane_id );
    void set_active( unsigned lane_id );
    // accessors
    virtual void print_insn(FILE *fp) const 
    {
        fprintf(fp," [inst @ pc=0x%04x] ", pc );
        for (int i=(int)m_config->warp_size-1; i>=0; i--)
            fprintf(fp, "%c", ((m_warp_active_mask[i])?'1':'0') );
    }
    bool active( unsigned thread ) const { return m_warp_active_mask.test(thread); }
    unsigned active_count() const { return m_warp_active_mask.count(); }
    unsigned issued_count() const { assert(m_empty == false); return m_warp_issued_mask.count(); }  // for instruction counting 
    bool empty() const { return m_empty; }
    void occupy() { m_empty=false; }
    unsigned warp_id() const 
    { 
        assert( !m_empty );
        return m_warp_id; 
    }
    unsigned get_warp_id() const
    {
        return m_warp_id;
    }
    void set_warp_id(unsigned warp_id)
    {
    	m_warp_id = warp_id;
    }
    unsigned dynamic_warp_id() const 
    { 
        assert( !m_empty );
        return m_dynamic_warp_id; 
    }
    bool has_callback( unsigned n ) const
    {
        return m_warp_active_mask[n] && m_per_scalar_thread_valid && 
            (m_per_scalar_thread[n].callback.function!=NULL);
    }
    new_addr_type get_addr( unsigned n ) const
    {
        assert( m_per_scalar_thread_valid );
        return m_per_scalar_thread[n].memreqaddr[0];
    }

    bool isatomic() const { return m_isatomic; }

    unsigned warp_size() const { return m_config->warp_size; }

    bool accessq_empty() const { return m_accessq.empty(); }
    unsigned accessq_count() const { return m_accessq.size(); }
    const mem_access_t &accessq_back() { return m_accessq.back(); }
    void accessq_pop_back() { m_accessq.pop_back(); }

    bool dispatch_delay()
    { 
        if( cycles > 0 ) 
            cycles--;
        return cycles > 0;
    }

    bool has_dispatch_delay(){
    	return cycles > 0;
    }

    void print( FILE *fout ) const;
    unsigned get_uid() const { return m_uid; }


protected:

    unsigned m_uid;
    bool m_empty;
    bool m_cache_hit;
    unsigned long long issue_cycle;
    unsigned cycles; // used for implementing initiation interval delay
    bool m_isatomic;
    bool m_is_printf;
    unsigned m_warp_id;
    unsigned m_dynamic_warp_id; 
    const core_config *m_config; 
    active_mask_t m_warp_active_mask; // dynamic active mask for timing model (after predication)
    active_mask_t m_warp_issued_mask; // active mask at issue (prior to predication test) -- for instruction counting

    struct per_thread_info {
        per_thread_info() {
            for(unsigned i=0; i<MAX_ACCESSES_PER_INSN_PER_THREAD; i++)
                memreqaddr[i] = 0;
        }
        dram_callback_t callback;
        new_addr_type memreqaddr[MAX_ACCESSES_PER_INSN_PER_THREAD]; // effective address, upto 8 different requests (to support 32B access in 8 chunks of 4B each)
    };
    bool m_per_scalar_thread_valid;
    std::vector<per_thread_info> m_per_scalar_thread;
    bool m_mem_accesses_created;
    std::list<mem_access_t> m_accessq;

    static unsigned sm_next_uid;
};

void move_warp( warp_inst_t *&dst, warp_inst_t *&src );

size_t get_kernel_code_size( class function_info *entry );



/* A 32 entries would suffice in hardware.
 * The extra entry is a side effect of current code assigns two *new*
 * entries on upon divergence rather than reusing the current entry
 * for one path and assigning only a single new entry.
 * TODO: recode the update function according to the above
 */
#define MAX_ST_SIZE 33

enum splits_table_entry_type {
    SPLITS_TABLE_ENTRY_TYPE_NORMAL = 0,
    SPLITS_TABLE_TYPE_CALL
};

struct simt_splits_table_entry {
    bool m_valid;
	bool m_blocked;
	bool m_virtual;
	bool m_transient;
	bool m_suspended;
	address_type m_pc;
    unsigned int m_calldepth;
    simt_mask_t m_active_mask;
    address_type m_recvg_pc;
    unsigned int m_recvg_entry;
    unsigned long long m_branch_div_cycle;
    splits_table_entry_type m_type;
    simt_splits_table_entry() :
    	m_valid(false), m_blocked(false),m_virtual(false),m_transient(false),m_suspended(false), m_pc(-1), m_calldepth(0), m_active_mask(), m_recvg_pc(-1), m_branch_div_cycle(0), m_type(SPLITS_TABLE_ENTRY_TYPE_NORMAL) {
    }
};

struct fifo_entry{
	bool m_blocked;
	unsigned m_st_entry;
	/*For statistics*/
	unsigned long long m_insertion_cycle;
	unsigned m_insertion_distance;

	fifo_entry(): m_blocked(false), m_st_entry(-1){}
	fifo_entry(unsigned num, unsigned long long cycle, unsigned dist): m_blocked(false), m_st_entry(num){
		m_insertion_cycle = cycle;
		m_insertion_distance = dist;
	}
	void update_insertion_cycle(unsigned long long cycle, unsigned dist){
		m_insertion_cycle = cycle;
		m_insertion_distance = dist;
	}
};

class simt_splits_table{
public:
    simt_splits_table( unsigned wid,  unsigned warpSize, const shader_core_config* config, const struct memory_config * mem_config, simt_tables * simt_table);
    void reset();
    void launch( address_type start_pc, const simt_mask_t &active_mask );
    unsigned insert_new_entry(address_type pc, address_type rpc, unsigned rpc_entry, const simt_mask_t & tmp_active_mask, splits_table_entry_type type, bool recvged=false);
    unsigned insert_new_entry(simt_splits_table_entry entry, bool recvged=false);
    bool fill_st_entry(unsigned entry);
    bool spill_st_entry();
    void get_pdom_splits_entry_info(unsigned num, unsigned *pc, unsigned *rpc );
    void get_pdom_active_split_info(unsigned *pc, unsigned *rpc );
    const simt_mask_t &get_active_mask(unsigned num);
    const simt_mask_t &get_active_mask();
    unsigned get_rpc();
    unsigned get_pc();
    unsigned get_rpc_entry();
    splits_table_entry_type get_type();
    bool valid();
    unsigned get_rpc(unsigned num);
    void invalidate();
    void update_active_entry();
    void update_pc(address_type new_pc);
    void set_to_blocked();
    void unset_blocked();
    void unset_blocked(unsigned entry);
    void release_blocked();
    bool is_blocked();
    bool is_virtual();
    bool is_blocked_or_virtual();
    bool split_reaches_barrier(address_type pc);
    void push_back();
    void push_back_once();
    unsigned  check_simt_splits_table();
    unsigned num_entries() {return m_num_entries;}
    unsigned getInsertionDist() {return m_fifo_queue.front().m_insertion_distance;}
    unsigned long long getInsertionCycle() {return m_fifo_queue.front().m_insertion_cycle;}
    void     print(FILE*fp);
    void cycle();
    bool branch_unit_avail() {return m_spill_st_entry.empty();}
    unsigned get_replacement_candidate();
    void set_shader(shader_core_ctx* shader);
    bool push_to_st_response_fifo(unsigned entry);
    bool is_virtualized();
    bool is_pending_reconvergence() {return m_pending_recvg_entry.m_valid;}
    bool st_space_available() {return m_num_physical_entries < m_max_st_size;}
    bool blocked();
    unsigned address_to_entry(warp_inst_t inst);

protected:
    unsigned m_warp_size;
    unsigned m_warp_id;
    unsigned m_max_st_size;
    unsigned m_num_entries;
    unsigned m_num_physical_entries;
    unsigned m_num_transient_entries;
    std::map<unsigned,simt_splits_table_entry> m_splits_table;
    std::deque<fifo_entry> m_fifo_queue;
    std::stack<int> m_invalid_entries;
    std::stack<int> m_available_v_id;
    unsigned m_active_split;
	warp_inst_t m_spill_st_entry;
	warp_inst_t m_fill_st_entry;
	int m_response_st_entry;
	shader_core_ctx* m_shader;
	const shader_core_config * m_config;
	const struct memory_config * m_mem_config;
	simt_tables* m_simt_tables;
	simt_splits_table_entry m_pending_recvg_entry;
};


#define MAX_RT_SIZE 32

struct simt_reconvergence_table_entry {
	bool m_valid;
	bool m_virtual;
	bool m_transient;
	address_type m_pc;
    unsigned int m_calldepth;
    simt_mask_t m_active_mask;
    simt_mask_t m_pending_mask;
    address_type m_recvg_pc;
    unsigned int m_recvg_entry;
    unsigned long long m_branch_rec_cycle;
    splits_table_entry_type m_type;
    simt_reconvergence_table_entry() :
    	m_valid(false), m_virtual(false), m_transient(false), m_pc(-1), m_calldepth(0), m_active_mask(), m_recvg_pc(-1), m_branch_rec_cycle(0) , m_type(SPLITS_TABLE_ENTRY_TYPE_NORMAL){
    };
};

class simt_reconvergence_table{
public:
    simt_reconvergence_table( unsigned wid,  unsigned warpSize,  const shader_core_config* config, const struct memory_config * m_mem_config, simt_tables * simt_table);
    void reset();
    const simt_mask_t & get_active_mask();
    const simt_mask_t & get_active_mask(unsigned num);
    void get_recvg_entry_info(unsigned num, unsigned *pc, unsigned *rpc );
    void get_active_recvg_info(unsigned *pc, unsigned *rpc );
    unsigned get_rpc(unsigned num);
    unsigned get_rpc();
    unsigned get_rpc_entry(unsigned num);
    splits_table_entry_type get_type(unsigned num);
    unsigned get_rpc_entry();
    unsigned get_pc(unsigned num);
    unsigned get_pc();
    void invalidate();
    void invalidate(unsigned num);
    bool update_pending_mask(unsigned top_recvg_entry,address_type top_recvg_pc,const simt_mask_t & tmp_active_mask, bool &suspended);
    unsigned insert_new_entry(address_type pc, address_type rpc, unsigned rpc_entry, const simt_mask_t & tmp_active_mask,splits_table_entry_type type);
	void update_masks_upon_time_out(unsigned k,const simt_mask_t & reconverged_mask);
	void set_rec_cycle(unsigned rec_entry,unsigned long long time);
    unsigned check_simt_reconvergence_table();
    simt_reconvergence_table_entry  get_recvg_entry(unsigned num);
    unsigned num_entries() {return m_num_entries;}
    void print (FILE *fout);
    void cycle();
    bool branch_unit_avail() {return m_spill_rec_entry.empty();}
    void set_shader(shader_core_ctx* shader) {m_shader=shader;}
    bool spill_rec_entry();
    bool fill_rec_entry(unsigned entry);
    bool is_pending_update() { return m_pending_update_entry.m_valid; }
    bool push_to_rt_response_fifo(unsigned entry);
    unsigned get_replacement_candidate();
    unsigned address_to_entry(warp_inst_t inst);

protected:
    unsigned m_warp_id;
    unsigned m_warp_size;
    unsigned m_num_entries;
    unsigned m_num_physical_entries;
    unsigned m_num_transient_entries;
    unsigned m_max_rec_size;
    std::map<int,simt_reconvergence_table_entry> m_recvg_table;
    std::stack<int> m_invalid_entries;
    unsigned m_active_reconvergence;
	const shader_core_config * m_config;
	shader_core_ctx* m_shader;
	simt_tables* m_simt_tables;
	warp_inst_t m_spill_rec_entry;
	warp_inst_t m_fill_rec_entry;
	int m_response_rec_entry;
	simt_reconvergence_table_entry m_pending_update_entry;

};


class simt_tables{
public:

    simt_tables( unsigned wid,  unsigned warpSize, const shader_core_config* config,const memory_config* mem_config);
    void reset();
    void launch( address_type start_pc, const simt_mask_t &active_mask );
    void update( simt_mask_t &thread_done, addr_vector_t &next_pc, address_type recvg_pc, op_type next_inst_op,unsigned next_inst_size, address_type next_inst_pc);
    const simt_mask_t &get_active_mask();
    void     get_pdom_active_split_info( unsigned *pc, unsigned *rpc );
    unsigned get_rp();
    void check_simt_tables();
    void check_time_out();
    void release_barrier();
    bool split_reaches_barrier(address_type pc);
    void     print(FILE*fp);
    unsigned getSTsize() {return m_simt_splits_table->num_entries();}
    unsigned getInsertionDist() {return m_simt_splits_table->getInsertionDist();}
    unsigned long long getInsertionCycle() {return m_simt_splits_table->getInsertionCycle();}
    unsigned getRTsize() {return m_simt_recvg_table->num_entries();}
    bool branch_unit_avail() {return m_simt_splits_table->branch_unit_avail() && m_simt_recvg_table->branch_unit_avail();}
    bool push_to_st_response_fifo(unsigned entry);
    bool push_to_rt_response_fifo(unsigned entry);
    void cycle()
    {
    	m_simt_splits_table->cycle();
    	m_simt_recvg_table->cycle();
    }
    void set_shader(shader_core_ctx* shader);
    void push_back();
    bool is_virtualized();
    bool is_pending_reconvergence();
    bool st_space_available();
    bool blocked();
    bool valid();
    bool is_blocked();
    bool fill_rec_entry(unsigned entry) {return m_simt_recvg_table->fill_rec_entry(entry);}
    bool insert_st_entry(address_type pc, address_type rpc, unsigned rpc_entry, const simt_mask_t & tmp_active_mask, splits_table_entry_type type, bool recvged=false)
    {
    	return m_simt_splits_table->insert_new_entry(pc,rpc,rpc_entry,tmp_active_mask,type,recvged);
    }
private:
    unsigned m_warp_id;
    unsigned m_warp_size;
	simt_splits_table* m_simt_splits_table;
	simt_reconvergence_table* m_simt_recvg_table;
	const shader_core_config * m_config;
	const struct memory_config * m_mem_config;
	shader_core_ctx * m_shader;
};

class simt_stack {
public:
    simt_stack( unsigned wid,  unsigned warpSize);

    void reset();
    void launch( address_type start_pc, const simt_mask_t &active_mask );
    void update( simt_mask_t &thread_done, addr_vector_t &next_pc, address_type recvg_pc, op_type next_inst_op,unsigned next_inst_size, address_type next_inst_pc );

    const simt_mask_t &get_active_mask() const;
    void     get_pdom_stack_top_info( unsigned *pc, unsigned *rpc ) const;
    unsigned get_rp() const;
    void     print(FILE*fp) const;

protected:
    unsigned m_warp_id;
    unsigned m_warp_size;

    enum stack_entry_type {
        STACK_ENTRY_TYPE_NORMAL = 0,
        STACK_ENTRY_TYPE_CALL
    };

    struct simt_stack_entry {
        address_type m_pc;
        unsigned int m_calldepth;
        simt_mask_t m_active_mask;
        address_type m_recvg_pc;
        unsigned long long m_branch_div_cycle;
        stack_entry_type m_type;
        simt_stack_entry() :
            m_pc(-1), m_calldepth(0), m_active_mask(), m_recvg_pc(-1), m_branch_div_cycle(0), m_type(STACK_ENTRY_TYPE_NORMAL) { };
    };

    std::deque<simt_stack_entry> m_stack;
};




/*
 * This abstract class used as a base for functional and performance and simulation, it has basic functional simulation
 * data structures and procedures. 
 */
class core_t {
    public:
        core_t( gpgpu_sim *gpu, 
                kernel_info_t *kernel,
                unsigned warp_size,
                unsigned threads_per_shader )
            : m_gpu( gpu ),
              m_kernel( kernel ),
              m_simt_stack( NULL ),
              m_simt_tables( NULL ),
              m_thread( NULL ),
              m_warp_size( warp_size )
        {
            m_warp_count = threads_per_shader/m_warp_size;
            // Handle the case where the number of threads is not a
            // multiple of the warp size
            if ( threads_per_shader % m_warp_size != 0 ) {
                m_warp_count += 1;
            }
            assert( m_warp_count * m_warp_size > 0 );
            m_thread = ( ptx_thread_info** )
                     calloc( m_warp_count * m_warp_size,
                             sizeof( ptx_thread_info* ) );
            initilizeSIMTDivergenceStructures(m_warp_count,m_warp_size);
            for(unsigned i=0; i<MAX_CTA_PER_SHADER; i++){
            	for(unsigned j=0; j<MAX_BARRIERS_PER_CTA; j++){
            		reduction_storage[i][j]=0;
            	}
            }

        }
        virtual ~core_t() { free(m_thread); }
        virtual void warp_exit( unsigned warp_id ) = 0;
        virtual bool warp_waiting_at_barrier( unsigned warp_id ) const = 0;
        virtual void checkExecutionStatusAndUpdate(warp_inst_t &inst, unsigned t, unsigned tid)=0;
        class gpgpu_sim * get_gpu() {return m_gpu;}
        void execute_warp_inst_t(warp_inst_t &inst, unsigned warpId =(unsigned)-1);
        bool  ptx_thread_done( unsigned hw_thread_id ) const ;
        void updateSIMTDivergenceStructures(unsigned warpId, warp_inst_t * inst);
        unsigned getSTSize(unsigned wid);
        unsigned getInsertionDist(unsigned wid);
        unsigned long long getInsertionCycle(unsigned wid);
        unsigned getRTSize(unsigned wid);
        void initilizeSIMTDivergenceStructures(unsigned warp_count, unsigned warps_size);
        void deleteSIMTDivergenceStructures();
        warp_inst_t getExecuteWarp(unsigned warpId);
        void get_pdom_stack_top_info( unsigned warpId, unsigned *pc, unsigned *rpc ) const;
        kernel_info_t * get_kernel_info(){ return m_kernel;}
        unsigned get_warp_size() const { return m_warp_size; }
        void and_reduction(unsigned ctaid, unsigned barid, bool value) { reduction_storage[ctaid][barid] &= value; }
        void or_reduction(unsigned ctaid, unsigned barid, bool value) { reduction_storage[ctaid][barid] |= value; }
        void popc_reduction(unsigned ctaid, unsigned barid, bool value) { reduction_storage[ctaid][barid] += value;}
        unsigned get_reduction_value(unsigned ctaid, unsigned barid) {return reduction_storage[ctaid][barid];}
    protected:
        class gpgpu_sim *m_gpu;
        kernel_info_t *m_kernel;
        simt_stack  **m_simt_stack; // pdom based reconvergence context for each warp
        simt_tables **m_simt_tables; // aware based reconvergence context for each warp (MIMD-Compatible)

        class ptx_thread_info ** m_thread;
        unsigned m_warp_size;
        unsigned m_warp_count;
        unsigned reduction_storage[MAX_CTA_PER_SHADER][MAX_BARRIERS_PER_CTA];
};


//register that can hold multiple instructions.
class register_set {
public:
	register_set(unsigned num, const char* name){
		for( unsigned i = 0; i < num; i++ ) {
			regs.push_back(new warp_inst_t());
		}
		m_name = name;
	}
	bool has_free(){
		for( unsigned i = 0; i < regs.size(); i++ ) {
			if( regs[i]->empty() ) {
				return true;
			}
		}
		return false;
	}
	bool has_ready(){
		for( unsigned i = 0; i < regs.size(); i++ ) {
			if( not regs[i]->empty() ) {
				return true;
			}
		}
		return false;
	}

	void move_in( warp_inst_t *&src ){
		warp_inst_t** free = get_free();
		move_warp(*free, src);
	}
	//void copy_in( warp_inst_t* src ){
		//   src->copy_contents_to(*get_free());
		//}
	void move_out_to( warp_inst_t *&dest ){
		warp_inst_t **ready=get_ready();
		move_warp(dest, *ready);
	}

	warp_inst_t** get_ready(){
		warp_inst_t** ready;
		ready = NULL;
		for( unsigned i = 0; i < regs.size(); i++ ) {
			if( not regs[i]->empty() ) {
				if( ready and (*ready)->get_uid() < regs[i]->get_uid() ) {
					// ready is oldest
				} else {
					ready = &regs[i];
				}
			}
		}
		return ready;
	}

	void print(FILE* fp) const{
		fprintf(fp, "%s : @%p\n", m_name, this);
		for( unsigned i = 0; i < regs.size(); i++ ) {
			fprintf(fp, "     ");
			regs[i]->print(fp);
			fprintf(fp, "\n");
		}
	}

	warp_inst_t ** get_free(){
		for( unsigned i = 0; i < regs.size(); i++ ) {
			if( regs[i]->empty() ) {
				return &regs[i];
			}
		}
		assert(0 && "No free registers found");
		return NULL;
	}

private:
	std::vector<warp_inst_t*> regs;
	const char* m_name;
};

#endif // #ifdef __cplusplus

#endif // #ifndef ABSTRACT_HARDWARE_MODEL_INCLUDED
