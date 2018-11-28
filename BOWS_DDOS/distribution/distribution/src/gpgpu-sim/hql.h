// Copyright (c) 2009-2011, Ahmed ElTantawy
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

/*
 * This is an implementation to HQL: A Scalable Synchronization Mechanism for GPUs.
 * Ayse Yilmazer, and David Kaeli.
 */

#ifndef HQL_INCLUDE
#define HQL_INCLUDE

#include <stdio.h>
#include <stdlib.h>

struct grant_status_queue_entry
{
	unsigned wid;
	unsigned numofgtw;
	bool newgrant;
};


enum cache_sync_status {
	VDNL = 0,	// Valid Data Not Locked
	IDNL,	// Invalid Data Not Locked
	IDNA,	// Invalid Data Not Allocated (needs a replacement candidate)
	IDLH,	// Invalid Data Lock Held
    IDLQ,	// Invalid Data Lock Queued
    IDLG,	// Invalid Data Lock Granted
    NUM_CACHE_SYNC_STATUS
};


struct l1_queue_entry{
	bool valid;
	unsigned wid;
	unsigned numAcquires;
};

struct l1_control_entry{
	unsigned NNE;
	unsigned cnt;
};

struct l1_locked_data_array{
	std::vector<l1_queue_entry> queue_entries;
	l1_control_entry control_entry;
};

struct tag_extension{
	bool locked;
	bool queued;
	bool granted;
	unsigned head;
	unsigned tail;
	bool NN;
};

#endif
