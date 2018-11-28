// Copyright (c) 2009-2011, Tor M. Aamodt, Ahmed ElTantawy, Tayler Hetherington
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


#include "gpu-cache.h"
#include "hql.h"

/*
enum cache_request_status
l2_cache::access_sync( new_addr_type addr,
                  mem_fetch *mf,
                  unsigned time,
                  std::list<cache_event> &events )
{
}
*/
/*
void tag_array::set_acquire_init_alloc( new_addr_type addr, unsigned idx ) const {
    cache_block_t *line = &m_lines[idx];
    line->m_status = VALID;
}
*/
/*
enum cache_sync_status tag_array::probe_sync( new_addr_type addr, unsigned &idx ) const {
    //assert( m_config.m_write_policy == READ_ONLY );
    unsigned set_index = m_config.set_index(addr);
    new_addr_type tag = m_config.tag(addr);

    unsigned invalid_line = (unsigned)-1;
    unsigned valid_line = (unsigned)-1;
    unsigned valid_timestamp = (unsigned)-1;

    bool all_reserved = true;
    bool invalid_line_found = false;
    unsigned invalid_line_idx = (unsigned)-1;
    // check for hit or pending hit
    for (unsigned way=0; way<m_config.m_assoc; way++) {
        unsigned index = set_index*m_config.m_assoc+way;
        cache_block_t *line = &m_lines[index];
        if (line->m_tag == tag) {
           if ( line->m_status == VALID ) {
                idx = index;
                if(line->m_tag_extension.granted)
                	return IDLG;
                if(line->m_tag_extension.queued)
                	return IDLQ;
                if(line->m_tag_extension.locked)
                	return IDLH;
                else
                	return VDNL;
            }else{
            	assert(line->m_status==INVALID);
            	invalid_line_found = true;
            	invalid_line_idx = index;
            }
        }
    }
    if(invalid_line){
    	idx = invalid_line_idx;
    	return IDNL;
    }
	return IDNA;
}


enum cache_request_status
l1_cache::access_sync( new_addr_type addr,
                  mem_fetch *mf,
                  unsigned time,
                  std::list<cache_event> &events )
{
	if(mf->is_acquire_init()){
	    new_addr_type block_addr = m_config.block_addr(addr);
	    unsigned cache_index = (unsigned)-1;
		enum cache_sync_status status = m_tag_array->probe_sync(block_addr,cache_index);
		printf("acquire init request from wid=%d to block=%d status=%d\n",mf->get_wid(),block_addr,status);
		if(status == IDNL){
			//look
		}else if(status == IDLG){
		}else if(status == IDLQ){
		}else if(status == IDLH){
		}else if(status == VDNL){

		}
	}
*/
    /*
     * Check if it is an aquire or release request
     * If acquire init request:
     * 	check of the line exists
     * 	if the line exists
     * 	 if the line locked
     * 	  assert that no entry for this wid
     * 	  add a new entry for this wid
     * 	  send L1acquire request to L2
     * 	 else
     * 	  evict the line
     * 	  add a new entry for this wid
     * 	  send an L1acquire request for L2
     * 	if the line does not exist
     * 	  if there is a replacement candidate
     * 	  	evict the candidate
     * 	  	add a new entry for this wid
     * 	  	send an L1acquire request for L2
     * 	  else
     * 	    return FAIL
     * If L1 aquiqre init ack:
     *   if L1acquire_retry:
     *   	return FAIL
     *   if L1acquire granted:
     *   	return Success
     *   if L1acquire_enqueueNN:
     *   	update id of NNE
     *   	send ack to L12
     *   if L1acquire_enqueued:
     */
/*
}
*/

