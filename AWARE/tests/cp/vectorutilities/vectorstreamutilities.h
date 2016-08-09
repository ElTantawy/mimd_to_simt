/////////////////////////////////////////////////////////////////////
//     (c) Electronic Arts 2007
// 
//! \author Andrew Brownsword (brownsword@ea.com)
//! \brief 
//! stream maniuplation functions for vectorutilities.h
//!
//! \ingroup miscellaneous vector functions
//! 

#ifndef INCLUDED_EA_VECTORSTREAMUTILITIES_H
#define INCLUDED_EA_VECTORSTREAMUTILITIES_H

#include "vectorutilities.h"

//- 4-way put functors (take 4 quadwords and 4 destination pointers) ----------------------------------------------------------

struct vec_put_float4_quartet {
	VEC_DECLARE_MEMORYINTERFACE;
	vec4fp zero;
	vec_put_float4_quartet () : zero((vec4fp)vec_immed_u32(0)) {}
	void operator() (unsigned int count, vec4fp src0, vec4fp src1, vec4fp src2, vec4fp src3, void* dst0, void* dst1, void* dst2, void* dst3)
	{
		vec_put_qword((vec4uint)src0, 0, dst0,  0);
		vec_put_qword((vec4uint)src1, 0, dst1,  0);
		vec_put_qword((vec4uint)src2, 0, dst2,  0);
		vec_put_qword((vec4uint)src3, 0, dst3,  0);
	}
};


//- Conversion from uncompressed in-order stream of quadwords to strided indexed output stream --------------------------------
// STOREFUNCTOR must be a "4-way put functor"

template<class STOREFUNCTOR, typename INDEXTYPE> void vec_convertquadwordstream_dstindexed(unsigned int count, INDEXTYPE* dstidx, void* src, size_t srcstride, void* dst, size_t dststride, STOREFUNCTOR functor)
{
	cacheprefetch(0, src);
	cacheprefetch(0, dstidx);

	unsigned char* srcbytes = (unsigned char*)src;
	unsigned char* dstbytes = (unsigned char*)dst;
	unsigned iter = count >> 2;

	for (unsigned int i = 0; i < iter; ++i)
	{
		cacheprefetch(128, srcbytes);
		cacheprefetch(128, dstidx);

		vec4fp src0 = vec_load(srcbytes, 0*srcstride);
		vec4fp src1 = vec_load(srcbytes, 1*srcstride);
		vec4fp src2 = vec_load(srcbytes, 2*srcstride);
		vec4fp src3 = vec_load(srcbytes, 3*srcstride);

		INDEXTYPE idx0 = *dstidx++;
		INDEXTYPE idx1 = *dstidx++;
		INDEXTYPE idx2 = *dstidx++;
		INDEXTYPE idx3 = *dstidx++;

		unsigned char* dstbytes0 = dstbytes + dststride * idx0;
		unsigned char* dstbytes1 = dstbytes + dststride * idx1;
		unsigned char* dstbytes2 = dstbytes + dststride * idx2;
		unsigned char* dstbytes3 = dstbytes + dststride * idx3;

		functor(4, src0, src1, src2, src3, dstbytes0, dstbytes1, dstbytes2, dstbytes3);

		srcbytes += 4*srcstride;
	}

	//write remainder
	unsigned int remainder = count - (iter << 2);
	if (remainder)
	{
		INDEXTYPE idx0, idx1, idx2;
		idx0 = idx1 = idx2 = dstidx[0];			// default all to first index

		vec4fp src0 = vec_load(srcbytes, 0);
		vec4fp src1 = src0, src2 = src0;		// default all to first value

		switch (remainder)
		{
		case 3:	src2 = vec_load(srcbytes, 2*srcstride);	idx2 = dstidx[2];	// fall through
		case 2:	src1 = vec_load(srcbytes, 1*srcstride);	idx1 = dstidx[1];	// fall through
		case 1:	break;
		}

		unsigned char* dstbytes0 = dstbytes + dststride * idx0;
		unsigned char* dstbytes1 = dstbytes + dststride * idx1;
		unsigned char* dstbytes2 = dstbytes + dststride * idx2;

		// strategy here to always write 4, but write same value repeatedly to same location if not all 4 writes are needed
		functor(remainder, src0, src1, src2, src0, dstbytes0, dstbytes1, dstbytes2, dstbytes0);
	}
}

//- Conversion from uncompressed index-order stream of quadwords to strided in-order output stream ----------------------------
// STOREFUNCTOR must be a "4-way put functor"

template<class STOREFUNCTOR, typename INDEXTYPE> void vec_convertquadwordstream_srcindexed(unsigned int count, INDEXTYPE* srcidx, void* src, size_t srcstride, void* dst, size_t dststride, STOREFUNCTOR functor)
{
	cacheprefetch(0, srcidx);

	unsigned char* srcbytes = (unsigned char*)src;
	unsigned char* dstbytes = (unsigned char*)dst;
	unsigned iter = count >> 2;

	for (unsigned int i = 0; i < iter; ++i)
	{
		cacheprefetch(128, srcidx);

		vec4fp src0 = vec_load(srcbytes, (*srcidx++)*srcstride);
		vec4fp src1 = vec_load(srcbytes, (*srcidx++)*srcstride);
		vec4fp src2 = vec_load(srcbytes, (*srcidx++)*srcstride);
		vec4fp src3 = vec_load(srcbytes, (*srcidx++)*srcstride);

		unsigned char* dstbytes0 = dstbytes;	dstbytes += dststride;
		unsigned char* dstbytes1 = dstbytes;	dstbytes += dststride;
		unsigned char* dstbytes2 = dstbytes;	dstbytes += dststride;
		unsigned char* dstbytes3 = dstbytes;	dstbytes += dststride;

		functor(4, src0, src1, src2, src3, dstbytes0, dstbytes1, dstbytes2, dstbytes3);
	}

	//write remainder
	unsigned int remainder = count - (iter << 2);
	if (remainder)
	{
		unsigned int mask1 = ~(0 - (numleadzeros32(remainder-1) >> 5));					// ~0 if remainder==1
		unsigned int mask2 = ~(0 - (numleadzeros32(remainder-2) >> 5)) && mask1;		// ~0 if remainder>2

		unsigned int index0 = *srcidx++;
		unsigned int index1 = int_select(index0, (unsigned int)*srcidx++, mask1);				// re-load from index0 if mask is not set
		unsigned int index2 = int_select(index0, (unsigned int)*srcidx++, mask2);				// re-load from index0 if mask is not set

		vec4fp src0 = vec_load(srcbytes, index0 * srcstride);
		vec4fp src1 = vec_load(srcbytes, index1 * srcstride);
		vec4fp src2 = vec_load(srcbytes, index2 * srcstride);

		unsigned char* dstbytes0 = dstbytes;																	dstbytes += dststride;
		unsigned char* dstbytes1 = (unsigned char*)int_select((uintptr_t)dstbytes0, (uintptr_t)dstbytes, mask1);	dstbytes += dststride;	// store to dstbytes0 if mask not set
		unsigned char* dstbytes2 = (unsigned char*)int_select((uintptr_t)dstbytes0, (uintptr_t)dstbytes, mask2);	dstbytes += dststride;	// store to dstbytes0 if mask not set

		// strategy here to always write 4, but write same value repeatedly to same location if not all 4 writes are needed
		functor(remainder, src0, src1, src2, src0, dstbytes0, dstbytes1, dstbytes2, dstbytes0);
	}
}


template<class STOREFUNCTOR, typename INDEXTYPE> void vec_transformconvertquadwordstream_srcindexed(unsigned int count, INDEXTYPE* srcidx, void *mat, void* src, size_t srcstride, void* dst, size_t dststride, STOREFUNCTOR functor)
{
	cacheprefetch(0, srcidx);

	unsigned char* srcbytes = (unsigned char*)src;
	unsigned char* dstbytes = (unsigned char*)dst;
	unsigned iter = count >> 2;

	vec4fp xrow = vec_load(mat, 0);
	vec4fp yrow = vec_load(mat, 16);
	vec4fp zrow = vec_load(mat, 32);
	vec4fp wrow = vec_load(mat, 48);

	for (unsigned int i = 0; i < iter; ++i)
	{
		cacheprefetch(128, srcidx);

		vec4fp src0 = vec_load(srcbytes, (*srcidx++)*srcstride);
		vec4fp src1 = vec_load(srcbytes, (*srcidx++)*srcstride);
		vec4fp src2 = vec_load(srcbytes, (*srcidx++)*srcstride);
		vec4fp src3 = vec_load(srcbytes, (*srcidx++)*srcstride);

		src0 = vec_transform_point(src0, xrow, yrow, zrow, wrow);
		src1 = vec_transform_point(src1, xrow, yrow, zrow, wrow);
		src2 = vec_transform_point(src2, xrow, yrow, zrow, wrow);
		src3 = vec_transform_point(src3, xrow, yrow, zrow, wrow);

		unsigned char* dstbytes0 = dstbytes;	dstbytes += dststride;
		unsigned char* dstbytes1 = dstbytes;	dstbytes += dststride;
		unsigned char* dstbytes2 = dstbytes;	dstbytes += dststride;
		unsigned char* dstbytes3 = dstbytes;	dstbytes += dststride;

		functor(4, src0, src1, src2, src3, dstbytes0, dstbytes1, dstbytes2, dstbytes3);
	}

	//write remainder
	unsigned int remainder = count - (iter << 2);
	if (remainder)
	{
		unsigned int index0 = *srcidx++;
		unsigned int index1 = (remainder > 1) ? (unsigned int)*srcidx++ : index0;
		unsigned int index2 = (remainder > 2) ? (unsigned int)*srcidx++ : index0;

		vec4fp src0 = vec_transform_point(vec_load(srcbytes, index0 * srcstride), xrow, yrow, zrow, wrow);
		vec4fp src1 = vec_transform_point(vec_load(srcbytes, index1 * srcstride), xrow, yrow, zrow, wrow);
		vec4fp src2 = vec_transform_point(vec_load(srcbytes, index2 * srcstride), xrow, yrow, zrow, wrow);

		unsigned char* dstbytes0 = dstbytes;									dstbytes += dststride;
		unsigned char* dstbytes1 = (remainder > 1) ? dstbytes : dstbytes0;		dstbytes += dststride;
		unsigned char* dstbytes2 = (remainder > 2) ? dstbytes : dstbytes0;		dstbytes += dststride;

		// strategy here to always write 4, but write same value repeatedly to same location if not all 4 writes are needed
		functor(remainder, src0, src1, src2, src0, dstbytes0, dstbytes1, dstbytes2, dstbytes0);
	}
}

#endif
