/////////////////////////////////////////////////////////////////////
//     (c) Electronic Arts 2007
// 
//! \author Andrew Brownsword (brownsword@ea.com)
//! \brief 
//! This is a collection of (somewhat) platform independent functions and
//! macros for accessing platform-specific SIMD intrinsics and types in a
//! generic way.  Note that the emphasis is entirely on performance and
//! cross-platform compatibility and usability are sacrificed where the 
//! would compromise performance.  Also note that these functions and macros
//! do not and cannot hide hardware factors which strongly impact the 
//! algorithm design, such as register counts, cache sizes, DMA usage, and
//! differences between instruction sets.
//!
//! The basic approach taken herein is to adopt the VMX intrinsic naming,
//! but extend it by adding _word, _short, _byte where specifying sizes
//! and _u32, _s32, _u16, etc. where specify precise types.  Float is the
//! usual default and has no extension.  These name changes are necessary
//! to support VMX128 (and SSE) where Microsoft chose to not provide a
//! typed vector, just a generic one, making intrinsic function overloading
//! impossible (alternatively, if you know how to achieve that let me know!).
//!
//! \ingroup miscellaneous vector types and functions
//! 
//! 

#ifndef INCLUDED_EA_VECTORUTILITIES_H
#define INCLUDED_EA_VECTORUTILITIES_H

// VECTOR_UTILITIES is defined on platforms with all the intrinsics
// VECTOR_UTILITIES_CORE is defined on all platforms but includes no intrinsics

// VECTOR_UTILITIES_SSE may be added in the future to define the subset that SSE can efficiently
// implement (SSE won't define VECTOR_UTILITIES, but VMX/VMX128/SPU will define VECTOR_UTILITIES_SSE)


// Memory & cache manipulation macros:  provided are four sets of macros are designed 
// to mask the differences between platforms.  Each covers a different use case.
//
// Low level cache macros:
//		cacheprefetch(offset, ptr)			issue a cacheline prefetch at the address (char*)ptr+offset, if appropriate to core
//		cachezero_start(p)					initialize the zeroing of an output stream to the specified location
//		cachezero_adv(p)					advance the zeroing of an output stream
//		cachezero_reset(p)					re-initializes the zeroing of an output stream to the specified location
//		cachezero_128()						zero one cacheline ahead in the output stream (note this may overrun end of buffer by 1 cacheline)
//		cachezero_256()						zero two cachelines ahead in the output stream (note this may overrun end of buffer by 2 cachelines)
//
// Translating pointers between address spaces:
//		VEC_COMPUTE_RELATIVE_POINTER(ptr, ptrbase, newbase)
//											returns a pointer relative to newbase, offset by same amount that ptr is from ptrbase
//											(simply returns ptr on platforms with only one address space)
// Keeping simple tables "local":
//		VEC_DECLARE_TABLECACHE(id, type, maxlen)	declares space for the cache of the table
//		VEC_CACHE_TABLE(id, ptr, length)			pull local table into the local cache, returns pointer to cached copy
//
// Simple input buffer pre-loading (different discrete buffers of varying type, size and address):
//		VEC_SETUP_INPUTBUFFER(type, ptr, bytes, limit)		pre-loads an input buffer
//			type:  buffer is array of this type
//			ptr:   start of buffer, must be 128 byte aligned; value of ptr may be altered
//			bytes: length of buffer in bytes
//			limit: maximum length of buffer, in bytes
//		VEC_WAIT_INPUTBUFFERS()				ensures that input buffers can now be used
//
//		Note that the id parameter for all macros below is a string that differentiates between instances so you can use these macros to
//		manage more than one instances (i.e. output buffer or cache) at a time.  For caches this is useful if the config parameters are 
//		different or if you don't want clients to compete for space.  For output buffers it allows more than one output.
//
// Read-only main memory cache:
//		VEC_SETUP_MAINMEMORYCACHE(id, blocksize, windowsize_pow2, blockcount, numignorebits, num_slots)		initializes cache
//			blocksize:  maximum number of bytes in a cache line, must be a multiple of 128
//			windowsize_pow2:  expressed as power of 2, rounds data pointers down by multiples of this to determine which line they go into... largest block must be no more than blocksize-windowsize
//			blockcount:	number of sets in the cache (sets * 4-way associativity == number of lines)
//			numignorebits:  number of bits to shift address down by before mapping into the cache's sets
//			num_slots:  number of distinct "clients" of the cache
//		VEC_FETCH_MAINMEMORY(id, ptr, index, size, slot)		retreive pointer to specified main memory address
//			ptr:  main memory address
//			index:  index into ptr buffer
//			size:  element size of contents of ptr buffer
//			slot:  0..num_slots-1, identifies which "client" of the cache this request is for
//			returns pointer to the fetched memory
//		VEC_USE_MAINMEMORY(id, slot)				advise cache that this client is going to use this the memory fetched to this slot
//		VEC_USE_MAINMEMORYSTREAM(id, ptr, slot)		as previous, but tracks progress through stream
//
//	Output buffer management (filling & writing of a series of discrete buffers at different addresses):
//		VEC_SETUP_OUTPUTBUFFER(id, buffercount, bufferbytes)	declare output buffers
//			buffercount:  number of cached buffers to allocate
//			bufferbytes:  size of buffers to allocate
//		VEC_GET_OUTPUTBUFFER(id, bStart, bSize, bOffset)		retrieve pointer to next output buffer
//			bStart:  main memory pointer to start of buffer
//			bSize:  number of bytes in output buffer
//			bOffset:  offset into region of output buffer to be updated next
//			returns pointer to the start of the output buffer (not to region to be updated!)
//		VEC_PREP_OUTPUTBUFFER(id)							prepare for writing to output buffer next target region
//		VEC_ADV_OUTPUTBUFFER(id, dst)						update output buffer with next output pointer
//		VEC_RESET_OUTPUTBUFFER(id, dst)						reset output buffer's notion of where data is going to
//		VEC_CLONE_OUTPUTBUFFER(id, ptr)						write an extra copy of the output buffer to the main memory location "ptr"
//		VEC_PURGE_OUTPUTBUFFER(id)							ensure output buffer is written to main memory (does NOT flush hardware cache)
//
//	Input stream management (contiguous uniform input streams):
//		VEC_SETUP_INPUTSTREAM(id, tag, type, xdramptr, srcaligned, count, periter, localptr, localcount)		declare input stream
//			tag: base DMA tag to use
//			type: type of stream content
//			xdramptr: main memory pointer to stream
//			srcaligned: guarantees that the source data is 128-byte aligned, cacheable, padded to a multiple of 128 and can be prefetched 1 cacheline beyond the end
//			count: number of type instances in stream (must be exact multiple of periter)
//			periter: number of type instances processed per loop iteration (must be compile time constant)
//			localptr: pointer to local buffer space to use (not used on cached architectures)
//			localcount: number of type instances that fit in localptr buffer
//		VEC_RESET_INPUTSTREAM(id)					reset stream to its initial condition; may NOT be called from inside processing loop
//		VEC_INPUTSTREAM_LOOP_1(id1)					loop control to process 1 stream
//		VEC_INPUTSTREAM_LOOP_2(id1,id2)				loop control to process 2 streams at once (count/periter must be the same for both)
//		VEC_INPUTSTREAM_LOOP_3(id1,id2,id3)			loop control to process 3 streams at once (count/periter must be the same for all)
//		VEC_INPUTSTREAM_LOOP_4(id1,id2,id3,id4)		loop control to process 4 streams at once (count/periter must be the same for all)
//		VEC_GETFROM_INPUTSTREAM(id)					retrieve pointer to periter type instances
//		VEC_ADVANCE_INPUTSTREAM(id)					if not using loop control for this stream, advances it to the next set of inputs
//		VEC_ATENDOF_INPUTSTREAM(id)					returns true if this stream has reached its end
//		VEC_CLOSE_INPUTSTREAM(id)					indicate that you are done with the input stream
//
//		Usage example 1:
//
//			VEC_SETUP_INPUTSTREAM(foo, 0, vec4fp, xdramptr1, 1, streamlen, 4, bufferptr1, buffercount1);
//			VEC_SETUP_INPUTSTREAM(bar, 0, float, xdramptr2, 1, streamlen*2, 8, bufferptr2, buffercount2);
//			VEC_INPUTSTREAM_LOOP_2(foo, bar)
//			{
//				vec4fp* vdata = VEC_GETFROM_INPUTSTREAM(foo);
//				float* fdata = VEC_GETFROM_INPUTSTREAM(bar);
//				... do work here ...
//			}
//			VEC_CLOSE_INPUTSTREAM(foo);
//			VEC_CLOSE_INPUTSTREAM(bar);
//
//		Usage example 2 (second stream doesn't take part in loop control):
//
//			VEC_SETUP_INPUTSTREAM(foo, 0, vec4fp, xdramptr1, 1, streamlen, 4, bufferptr1, buffercount1);
//			VEC_SETUP_INPUTSTREAM(bar, 0, float, xdramptr2, 1, streamlen*2, 8, bufferptr2, buffercount2);
//			VEC_INPUTSTREAM_LOOP_1(foo)
//			{
//				vec4fp* vdata = VEC_GETFROM_INPUTSTREAM(foo);
//				float* fdata = VEC_ATENDOF_INPUTSTREAM(bar) ? VEC_GETFROM_INPUTSTREAM(bar) : data_to_use_when_bar_is_empty;
//				... do work here ...
//				if (some condition)
//					VEC_ADVANCE_INPUTSTREAM(bar);
//			}
//			VEC_CLOSE_INPUTSTREAM(foo);
//			VEC_CLOSE_INPUTSTREAM(bar);
//			
//	Output stream management (contiguous uniform output streams):
//		VEC_SETUP_OUTPUTSTREAM(id, tag, type, xdramptr, padded, periter, localptr, localcount)		declare output stream
//			tag: base DMA tag to use
//			type: type of stream content
//			xdramptr: main memory pointer to stream
//			padded: is a compile time 0/1 flag indicating whether the output stream can assume that it has at least 1 cacheline of padding beyond the element written to it
//			periter: maximum number of type instances processed per loop iteration (must be compile time constant)
//			localptr: pointer to local buffer space to use (not used on cached architectures)
//			localcount: number of type instances that fit in localptr buffer
//		VEC_RESET_OUTPUTSTREAM(id)					reset stream to its initial condition; may NOT be called from inside processing loop
//		VEC_GETFOR_OUTPUTSTREAM(id)					retrieve output pointer to space for periter type instances, must call VEC_PUT_OUTPUTSTREAM between calls to GETFOR
//		VEC_PUT_OUTPUTSTREAM(id, count)				write count elements to the output stream from the last pointer returned from GETFOR
//													0 <= count <= periter, and it becomes illegal to write to the last pointer returned from GETFOR
//		VEC_CLOSE_OUTPUTSTREAM(id)					ensures that all data has been written

#include <string.h>
#include <stdint.h>

template<bool> struct vec_compiletimecheck { vec_compiletimecheck(...); };
template<> struct vec_compiletimecheck<false> {};
#define VEC_CT_ASSERT(expr, msg) { class ERROR_##msg {}; (void)sizeof( (vec_compiletimecheck<(expr)>(ERROR_##msg()))); }

#ifdef EA_PLATFORM_XENON
#define VECTOR_UTILITIES 1
#define VECTOR_UTILITIES_CORE 1
#include "vectorutilities_xenon.h"
#elif defined(EA_PLATFORM_PS3)
#define VECTOR_UTILITIES 1
#define VECTOR_UTILITIES_CORE 1
#include "vectorutilities_ppu.h"
#elif defined(EA_PLATFORM_PS3_SPU)
#define VECTOR_UTILITIES 1
#define VECTOR_UTILITIES_CORE 1
#include "vectorutilities_spu.h"
#elif defined(EA_PLATFORM_WII)
// VECTOR_UTILITIES not defined
#define VECTOR_UTILITIES_CORE 1
#include "vectorutilities_wii.h"
#else
// VECTOR_UTILITIES not defined
#define VECTOR_UTILITIES_CORE 1
#include "vectorutilities_generic.h"
#endif

	// Platform independent section

#ifdef VECTOR_UTILITIES
	// This macro should be called to provide the constants required by many of the support functions below
	#define VEC_GENERAL_SETUP()								\
		vec4uint integer1 = vec_immed_u32(1);				\
		vec4fp v0 = (vec4fp)vec_immed_s32(0);				\
		vec4fp v1 = vec_ctfu(integer1, 0);
#endif

// default to turning prints off
#ifndef VEC_PRINTF
#define VEC_PRINTF vec_printf
inline void vec_printf(const char* fmt, ...) {};
#endif

// default to turning asserts off
#ifndef VEC_ASSERT
#define VEC_ASSERT(cond) 
#endif

#endif
