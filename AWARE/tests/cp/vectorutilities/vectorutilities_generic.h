/////////////////////////////////////////////////////////////////////
//     (c) Electronic Arts 2007
// 
//! \author Andrew Brownsword (brownsword@ea.com)
//! \brief 
//! Generic C partial implementation for vectorutilities.h
//!
//! \ingroup miscellaneous vector types and functions
//! 

#ifndef INCLUDED_EA_VECTORUTILITIES_GENERIC_H
#define INCLUDED_EA_VECTORUTILITIES_GENERIC_H

#ifndef VEC_ASSERT
#define VEC_ASSERT(cond) 
#endif

#ifndef VEC_PRINTF
#define VEC_PRINTF vec_printf
inline void vec_printf(const char* fmt, ...) {}
#endif

struct vec4fp { float v[4]; template<typename TYPE> operator TYPE () { return *(TYPE*)this; }; };
struct vec2long { int64_t v[4]; template<typename TYPE> operator TYPE () { return *(TYPE*)this; }; };
struct vec4ulong { uint64_t v[4]; template<typename TYPE> operator TYPE () { return *(TYPE*)this; }; };
struct vec4int { int32_t v[4]; template<typename TYPE> operator TYPE () { return *(TYPE*)this; }; };
struct vec4uint { uint32_t v[4]; template<typename TYPE> operator TYPE () { return *(TYPE*)this; }; };
struct vec8short { int16_t v[8]; template<typename TYPE> operator TYPE () { return *(TYPE*)this; }; };
struct vec8ushort { uint16_t v[8]; template<typename TYPE> operator TYPE () { return *(TYPE*)this; }; };
struct vec16char { int8_t v[16]; template<typename TYPE> operator TYPE () { return *(TYPE*)this; }; };
struct vec16byte { uint8_t v[16]; template<typename TYPE> operator TYPE () { return *(TYPE*)this; }; };

#define VEC_EXPECT(cond, predict)	(cond)

inline int numleadzeros32(uint32_t v)
{
	uint32_t mask = ~0U;
	int bits = -1;
	do { mask >>= 1; bits += 1; } while (((v & mask) == 0) && (bits < 32));
	return bits;
}

inline int numleadzeros64(uint64_t v)
{
	uint64_t mask = ~0ULL;
	int bits = -1;
	do { mask >>= 1; bits += 1; } while (((v & mask) == 0) && (bits < 64));
	return bits;
}

#define numleadzeros64(v)	__cntlzd(v)

#define int_select(a,b,m)		(((b) & (m)) | ((a) & ~(m)))
#define float_select(cmp, a,b)	(((cmp) >= 0.0)?(a):(b))
inline uint32_t load_unaligned_uint32(const void* ptr){ return *(uint32_t*)(ptr); };

inline vec4fp vec_ctfu (vec4uint v, uint32_t b) { float multiplier = 1.0f / (float)(1 << b); vec4fp result; for (int i=0;i<4;++i) result.v[i] = v.v[i]*multiplier; return result; };
inline vec4fp vec_ctfs (vec4int v, uint32_t b) { float multiplier = 1.0f / (float)(1 << b); vec4fp result; for (int i=0;i<4;++i) result.v[i] = v.v[i]*multiplier; return result; };
inline vec4uint vec_ctu (vec4fp v, uint32_t b) { float multiplier = (float)(1 << b); vec4uint result; for (int i=0;i<4;++i) result.v[i] = (uint32_t)(v.v[i]*multiplier); return result; };
inline vec4int vec_cts (vec4fp v, uint32_t b) { float multiplier = (float)(1 << b); vec4int result; for (int i=0;i<4;++i) result.v[i] = (int32_t)(v.v[i]*multiplier); return result; };

template<typename TYPE, int N> TYPE vec_splat_n (const TYPE& v, uint32_t idx) 
{ 
	assert(idx<N); 
	TYPE result; 
	for (int i=0;i<N;++i) 
		result.v[i] = v.v[idx]; 
	return result; 
};

template<typename TYPE> TYPE vec_splat_word (const TYPE& v, int idx) { return vec_splat_n<TYPE,4>(v,idx); };
template<typename TYPE> TYPE vec_splat_short (const TYPE& v, int idx) { return vec_splat_n<TYPE,8>(v,idx); };
template<typename TYPE> TYPE vec_splat_byte (const TYPE& v, int idx) { return vec_splat_n<TYPE,16>(v,idx); };

vec4int vec_immed_s32 (int32_t s) { vec4int result; for (int i=0;i<4;++i) result.v[i] = s; return result; };
vec8short vec_immed_s16 (int16_t s) { vec8short result; for (int i=0;i<8;++i) result.v[i] = s; return result; };
vec16char vec_immed_s8 (int8_t s) { vec16char result; for (int i=0;i<16;++i) result.v[i] = s; return result; };
vec4uint vec_immed_u32 (uint32_t s) { vec4uint result; for (int i=0;i<4;++i) result.v[i] = s; return result; };
vec8ushort vec_immed_u16 (uint16_t s) { vec8ushort result; for (int i=0;i<8;++i) result.v[i] = s; return result; };
vec16byte vec_immed_u8 (uint8_t s) { vec16byte result; for (int i=0;i<16;++i) result.v[i] = s; return result; };

//#define vec_unpacklh		vec_unpackl
//#define vec_unpackhh		vec_unpackh
//#define vec_packw			vec_pack
//#define vec_packh			vec_pack
//#define vec_packs_sw_s		vec_packs
//#define vec_packs_sh_s		vec_packs
//#define vec_packs_sw_u		vec_packsu
//#define vec_packs_sh_u		vec_packsu
//#define vec_packs_uw		vec_packu
//#define vec_packs_uh		vec_packu
template<typename TYPE> TYPE vec_sld (TYPE a, TYPE b, int c)
{
	TYPE r;
	vec16byte *aptr = (vec16byte*)&a;
	vec16byte *bptr = (vec16byte*)&b;
	vec16byte *rptr = (vec16byte*)&r;
	for (int i=0;i<16;++i) 
		rptr->v[i] = ((i+c) < 16) ? aptr->v[i+c] : bptr->v[i+c-16];
	return r;
}

//vec_srl
//#define vec_sl_word			vec_sl
//#define vec_sr_word(a,b)	vec_sr((vec4uint)a, b)
//#define vec_sra_word(a,b)	vec_sra((vec4int)a, b)
//#define vec_rl_word			vec_rl
//#define vec_add_u32			vec_add
//#define vec_sub_u32			vec_sub

template<typename TYPE> TYPE vec_perm (TYPE a, TYPE b, vec16byte c)
{
	TYPE r;
	vec16byte *aptr = (vec16byte*)&a;
	vec16byte *bptr = (vec16byte*)&b;
	vec16byte *rptr = (vec16byte*)&r;
	for (int i=0;i<16;++i) 
		rptr->v[i] = ((c.v[i]&0x1f) < 16) ? aptr->v[c.v[i]&0xf] : bptr->v[c.v[i]&0xf];
	return r;
}

template<typename TYPE> vec4uint vec_select (TYPE a, TYPE b, vec4uint c)
{
	vec4uint result;
	vec4uint* aptr = (vec4uint*)&a;
	vec4uint* bptr = (vec4uint*)&b;
	for(int i=0;i<4;++i)
		result.v[i] = int_select(a->v[i], b->v[i], c.v[i]);
	return result;
}

inline vec4fp vec_abs(vec4fp a)
{
	vec4fp result;
	for (int i=0;i<4;++i) result.v[i] = fabsf(a);
	return result;
}

template<typename TYPE> TYPE vec_min (TYPE a, TYPE b)
{
	TYPE result;
	size_t n = sizeof(TYPE) / sizeof(a.v[0]);
	for(int i=0;i<n;++i)
		result.v[i] = (a.v[i] < b.v[i]) ? a.v[i] : b.v[i];
	return result;
}

template<typename TYPE> TYPE vec_max (TYPE a, TYPE b)
{
	TYPE result;
	size_t n = sizeof(TYPE) / sizeof(a.v[0]);
	for(int i=0;i<n;++i)
		result.v[i] = (a.v[i] > b.v[i]) ? a.v[i] : b.v[i];
	return result;
}

inline vec4fp vec_madd (vec4fp a, vec4fp b, vec4fp c)
{
	vec4fp result;
	for(int i=0;i<4;++i)
		result.v[i] = a.v[i]*b.v[i]+c.v[i];
	return result;
}

inline vec4fp vec_nmsub (vec4fp a, vec4fp b, vec4fp c)
{
	vec4fp result;
	for(int i=0;i<4;++i)
		result.v[i] = -(a.v[i]*b.v[i]-c.v[i]);
	return result;
}

inline vec4fp vec_mult (vec4fp a, vec4fp b)
{
	vec4fp result;
	for(int i=0;i<4;++i)
		result.v[i] = a.v[i]*b.v[i];
	return result;
}

template<typename TYPE> TYPE vec_add (TYPE a, TYPE b)
{
	TYPE result;
	size_t n = sizeof(TYPE) / sizeof(a.v[0]);
	for(int i=0;i<n;++i)
		result.v[i] = a.v[i]+b.v[i];
	return result;
}

template<typename TYPE> TYPE vec_sub (TYPE a, TYPE b)
{
	TYPE result;
	size_t n = sizeof(TYPE) / sizeof(a.v[0]);
	for(int i=0;i<n;++i)
		result.v[i] = a.v[i]-b.v[i];
	return result;
}

#define vec_msub(a,b,c)		vec_sub(vec_madd(a, b, (vec4fp)vec_splat_u32(0)), c)
#define vec_subm_u32		vec_sub
#define vec_subs_u32		vec_sub

template<typename TYPE> TYPE vec_and (TYPE a, TYPE b)
{
	TYPE result;
	size_t n = sizeof(TYPE) / sizeof(a.v[0]);
	for(int i=0;i<n;++i)
		result.v[i] = a.v[i] & b.v[i];
	return result;
}

template<typename TYPE> TYPE vec_andc (TYPE a, TYPE b)
{
	TYPE result;
	size_t n = sizeof(TYPE) / sizeof(a.v[0]);
	for(int i=0;i<n;++i)
		result.v[i] = a.v[i] & ~b.v[i];
	return result;
}

template<typename TYPE> TYPE vec_or (TYPE a, TYPE b)
{
	TYPE result;
	size_t n = sizeof(TYPE) / sizeof(a.v[0]);
	for(int i=0;i<n;++i)
		result.v[i] = a.v[i] | b.v[i];
	return result;
}

template<typename TYPE> TYPE vec_xor (TYPE a, TYPE b)
{
	TYPE result;
	size_t n = sizeof(TYPE) / sizeof(a.v[0]);
	for(int i=0;i<n;++i)
		result.v[i] = a.v[i] ^ b.v[i];
	return result;
}

#define vec_min_u32			vec_min

inline vec4fp vec_msum3fp(vec4fp a, vec4fp b)	
{
	float sum = 0.0f;
	for (int i=0;i<3;++i)
		sum += a.v[i]*b.v[i];
	vec4fp result;
	for (int i=0;i<4;++i)
		result.v[i] = sum;
	return result;	
};

inline vec4fp vec_msum4fp(vec4fp a, vec4fp b)	
{
	float sum = 0.0f;
	for (int i=0;i<4;++i)
		sum += a.v[i]*b.v[i];
	vec4fp result;
	for (int i=0;i<4;++i)
		result.v[i] = sum;
	return result;	
};

template<typename TYPE> TYPE vec_mergeh(TYPE a, TYPE b)
{
	TYPE result;
	size_t n = sizeof(TYPE) / sizeof(a.v[0]);
	int iter = n/2;
	for (int i=0; i<iter; i += 1)
	{
		result.v[i*2+0] = a.v[i];
		result.v[i*2+1] = b.v[i];
	}
	return result;
}

template<typename TYPE> TYPE vec_mergel(TYPE a, TYPE b)
{
	TYPE result;
	size_t n = sizeof(TYPE) / sizeof(a.v[0]);
	int iter = n/2;
	for (int i=0; i<iter; i += 1)
	{
		result.v[iter+i*2+0] = a.v[i];
		result.v[iter+i*2+1] = b.v[i];
	}
	return result;
}

#define vec_mergelw(a,b)	vec_mergel((vec4uint)(a), (vec4uint)(b))
#define vec_mergehw(a,b)	vec_mergeh((vec4uint)(a), (vec4uint)(b))
#define vec_mergelh(a,b)	vec_mergel((vec8ushort)(a), (vec8ushort)(b))
#define vec_mergehh(a,b)	vec_mergeh((vec8ushort)(a), (vec8ushort)(b))

//vec_cmpgt
//vec_cmpeq
//vec_cmpge

//#define vec_cmpgt_u32		vec_cmpgt
//#define vec_cmpgt_u16		vec_cmpgt
//#define vec_cmpgt_u8		vec_cmpgt

//#define vec_cmpgt_s32		vec_cmpgt
//#define vec_cmpgt_s16		vec_cmpgt
//#define vec_cmpgt_s8		vec_cmpgt

//#define vec_cmpeq_u32		vec_cmpeq
//#define vec_cmpeq_u16		vec_cmpeq
//#define vec_cmpeq_u8		vec_cmpeq

//#define vec_cmpeq_s32		vec_cmpeq
//#define vec_cmpeq_s16		vec_cmpeq
//#define vec_cmpeq_s8		vec_cmpeq

/* inline int vec_any_nonzero(vec4fp a)
{
	return vec_any_ne( a, (vec4fp) vec_immed_u32( 0u ) );
}
 */

//- fast integer min/max/select -------------------------------------------------------------
template<typename TYPE>  TYPE fast_minint (TYPE x, TYPE y)
{
	int64_t delta = (int64_t)x - (int64_t)y;
	TYPE result = (y + (delta & (delta >> (sizeof(TYPE) * 8 - 1))));
	xASSERT(result == std::min(x,y));
	return(result);
}

template<typename TYPE>  TYPE fast_maxint (TYPE x, TYPE y)
{
	int64_t delta = (int64_t)x - (int64_t)y;
	TYPE result = (x - (delta & (delta >> (sizeof(TYPE) * 8 - 1))));
	return(result);
}

template<typename TYPE, typename COND>  TYPE int_selectif (TYPE a, TYPE b, COND c)
{
	uint64_t m = (c)?~0ULL:0ULL;
	return(int_select(a, b, (TYPE)m));
}

//- templated buffer object -----------------------------------------------------------------
template<typename TYPE, int LENGTH=1> struct TemplateBuffer
{
public:
	TemplateBuffer (void) {};
	TemplateBuffer (const TYPE& src) { for (int i = 0; i < LENGTH; ++i) mData[i] = src; };
	~TemplateBuffer (void) {};
	TYPE* GetDataPointer (void) { return(mData); };
	size_t GetDataSize (void) { return(LENGTH * sizeof(TYPE)); };
private:
	TYPE	mData[LENGTH];
	char	mPad[((128 - sizeof(TYPE)*LENGTH) & 127)+256];	// pad to full cacheline, and add two more cachelines
};

//- cache prefetch macros -------------------------------------------------------------------
#define cacheprefetch(ofs, ptr)	
#define cachezero(ofs, ptr) 
#define cachezero_start(p)
#define cachezero_adv(p)
#define cachezero_reset(p)
#define cachezero_128()
#define cachezero_256()

#ifndef VEC_SPUHEAP_DECLARE
#define VEC_SPUHEAP_DECLARE(stacksize)
#define VEC_SPUHEAP_BEGIN()							NULL
#define VEC_SPUHEAP_END()							NULL
#define VEC_SPUHEAP_SIZE()							0
#define VEC_SPUHEAP_RESERVE(bytes)					/* no op */
#define VEC_SPUHEAP_VALIDATE()						/* no op */
#define VEC_SPUHEAP_RESERVEALIGNED(bytes, alignto)	NULL
#endif

//- buffer management -----------------------------------------------------------------------
#define VEC_COMPUTE_RELATIVE_POINTER(ptr, ptrbase, newbase)	(ptr)

#define VEC_DECLARE_TABLECACHE(id, type, maxlen)
#define VEC_CACHE_TABLE(id, ptr, length)			(ptr)

#define VEC_SETUP_INPUTBUFFER(type, ptr, bytes, limit)					
#define VEC_MANUAL_INPUTBUFFER(type, xdramAddr, localAddr, elements)	VEC_ASSERT((xdramAddr == localAddr) || !localAddr)
#define VEC_WAIT_INPUTBUFFERS()											/* do nothing */


#define VEC_SETUP_MAINMEMORYCACHE(id, blocksize, windowsize_pow2, blockcount, numignorebits, num_slots)
#define VEC_FETCH_MAINMEMORY(id, ptr, index, size, slot) ((const char*)ptr + index * size)
#define VEC_USE_MAINMEMORY(id, slot)
#define VEC_USE_MAINMEMORYSTREAM(id, ptr, slot)

#define VEC_SETUP_OUTPUTBUFFER(id, buffercount, bufferbytes)											\
	unsigned int prev_output_bytes##id = 0;																\
	void * prev_output_buffer##id = NULL
#define VEC_GET_OUTPUTBUFFER(id, bStart, bSize, bOffset)												\
	bStart;	/* return value of macro */																	\
	VEC_ASSERT((prev_output_buffer##id != bStart));														\
	prev_output_buffer##id = bStart;																	\
	prev_output_bytes##id = bSize*sizeof(*bStart)
#define VEC_PREP_OUTPUTBUFFER(id)
#define VEC_ADV_OUTPUTBUFFER(id, dst)
#define VEC_RESET_OUTPUTBUFFER(id, dst)
#define VEC_CLONE_OUTPUTBUFFER(id, ptr) std::memcpy(ptr, (char*)prev_output_buffer##id, prev_output_bytes##id)
#define VEC_PURGE_OUTPUTBUFFER(id)

#define VEC_SETUP_INPUTSTREAM(id, tag, type, xdramptr, srcaligned, count, periter, localptr, localcount)\
	type *private_inputstream_##id = (type *)xdramptr;													\
	type *private_inputstream_base_##id = (type *)xdramptr;												\
	int private_loopiter_##id = periter;																\
	int private_loopcount_##id = count;																	\
	int private_loopcounter_##id = 0
#define VEC_GETFROM_INPUTSTREAM(id)																		\
	private_inputstream_##id
#define VEC_ADVANCE_INPUTSTREAM(id)																		\
	private_inputstream_##id += private_loopiter_##id;													\
	private_loopcounter_##id += 1
#define VEC_RESET_INPUTSTREAM(id)																		\
	private_inputstream_##id = private_inputstream_base_##id;											\
	private_loopcounter_##id = 0
#define VEC_CLOSE_INPUTSTREAM(id)																		\
	private_inputstream_base_##id = NULL; private_loopcounter_##id = private_loopcount_##id
#define VEC_ATENDOF_INPUTSTREAM(id)																		\
	(private_loopcounter_##id == private_loopcount_##id)

#define VEC_INPUTSTREAM_LOOP_1(id1)																		\
	for (private_loopcounter_##id1 = 0; private_loopcounter_##id1 < private_loopcount_##id1;			\
		private_loopcounter_##id1 += private_loopiter_##id1,											\
		private_inputstream_##id1 += private_loopiter_##id1)
#define VEC_INPUTSTREAM_LOOP_2(id1,id2)																	\
	for (private_loopcounter_##id1 = 0; private_loopcounter_##id1 < private_loopcount_##id1;			\
		 ++private_loopcounter_##id1, private_inputstream_##id1 += private_loopiter_##id1,				\
		 ++private_loopcounter_##id2, private_inputstream_##id2 += private_loopiter_##id2)
#define VEC_INPUTSTREAM_LOOP_3(id1,id2,id3)																\
	for (private_loopcounter_##id1 = 0; private_loopcounter_##id1 < private_loopcount_##id1;			\
		 ++private_loopcounter_##id1, private_inputstream_##id1 += private_loopiter_##id1,				\
		 ++private_loopcounter_##id2, private_inputstream_##id2 += private_loopiter_##id2,				\
		 ++private_loopcounter_##id3, private_inputstream_##id3 += private_loopiter_##id3)
#define VEC_INPUTSTREAM_LOOP_4(id1,id2,id3,id4)															\
	for (private_loopcounter_##id1 = 0;  private_loopcounter_##id1 < private_loopcount_##id1;			\
		 ++private_loopcounter_##id1, private_inputstream_##id1 += private_loopiter_##id1,				\
		 ++private_loopcounter_##id2, private_inputstream_##id2 += private_loopiter_##id2,				\
		 ++private_loopcounter_##id3, private_inputstream_##id3 += private_loopiter_##id3,				\
		 ++private_loopcounter_##id4, private_inputstream_##id4 += private_loopiter_##id4)


#define VEC_SETUP_OUTPUTSTREAM(id, tag, type, xdramptr, padded, periter, localptr, localcount)			\
	type* private_outputstream_base##id = xdramptr;														\
	type* private_outputstream_ptr##id = xdramptr
#define VEC_RESET_OUTPUTSTREAM(id)																		\
	private_outputstream_ptr##id = private_outputstream_base##id
#define VEC_GETFOR_OUTPUTSTREAM(id)																		\
	private_outputstream_ptr##id
#define VEC_PUT_OUTPUTSTREAM(id, count)																	\
	private_outputstream_ptr##id += count
#define VEC_CLOSE_OUTPUTSTREAM(id)	/* do nothing */

//- local memory load/store -----------------------------------------------------------------

template<typename VECTYPE> void vec_store (VECTYPE data, void* dst, size_t offset)
{
	memcpy(dst, &data, sizeof(data));
}

inline vec4fp vec_load (const void* src, size_t offset)
{
	vec4fp result;
	memcpy(&result, (const char*)src + offset, sizeof(result));
	return result;
}

inline vec4fp vec_loadunaligned (const void* src)
{
	return vec_load(src, 0);
}

inline vec4uint vec_loadsplatword (const void* src)
{
	const uint32_t* srcintptr = (const uint32_t*)src;
	vec4uint result;
	for (int i=0;i<4;++i)
		result.v[i] = *srcintptr;
	return(result);
}

inline vec8ushort vec_loadsplatushort (const unsigned short* src)
{
	vec8ushort result;
	for (int i=0;i<8;++i)
		result.v[i] = *src;
	return(result);
}

inline vec8short vec_loadsplatshort (const short* src)
{
	vec8short result;
	for (int i=0;i<8;++i)
		result.v[i] = *src;
	return(result);
}

//- main memory load/store ------------------------------------------------------------------

#define VEC_DECLARE_MEMORYINTERFACE	/* nothing to do here */

	 void vec_put_qword (const vec4uint& data, unsigned int index, void* dst, unsigned int offset)
	{
		uintptr_t dstptr = ((uintptr_t)dst & ~0xf) + offset;
		*(vec4uint*)dstptr = data;
	}

	 void vec_put_dword (const vec4uint& data, unsigned int index, void* dst, unsigned int offset)
	{
		uintptr_t dstptr = ((uintptr_t)dst & ~0xf) + offset;
		vec4ulong* dstvec = (vec4ulong*)dstptr;
		vec4ulong ldata = (const vec4ulong&)data;
		dstvec[index].v[index] = ldata.v[index];
	}

	 void vec_put_word (const vec4uint& data, unsigned int index, void* dst, unsigned int offset)
	{
		uintptr_t dstptr = ((uintptr_t)dst & ~0xf) + offset;
		vec4uint* dstvec = (vec4uint*)dstptr;
		dstvec->v[index] = data.v[index];
	}

	 void vec_put_half (const vec8ushort& data, unsigned int index, void* dst, unsigned int offset)
	{
		uintptr_t dstptr = ((uintptr_t)dst & ~0xf) + offset;
		vec8ushort* dstvec = (vec8ushort*)dstptr;
		dstvec->v[index] = data.v[index];
	}

	 void vec_put_byte (const vec16byte& data, unsigned int index, void* dst, unsigned int offset)
	{
		uintptr_t dstptr = ((uintptr_t)dst & ~0xf) + offset;
		vec16byte* dstvec = (vec16byte*)dstptr;
		dstvec->v[index] = data.v[index];
	}

//- math support ----------------------------------------------------------------------------

inline vec4uint vec_int2vectormask (unsigned int mask)
{
	vec4uint result;
	for (int i=0;i<4;++i) result.v[i] = mask;
	return result;
}

//vec_all_eq
//vec_any_gt

inline void vec_transpose (vec4fp _a, vec4fp _b, vec4fp _c, vec4fp _d,
									vec4fp& a, vec4fp& b, vec4fp& c, vec4fp& d)
{	// transpose 4 vector4 into 4 vector4
	vec4fp transpose1 = vec_mergeh( _a, _c );
	vec4fp transpose2 = vec_mergeh( _b, _d );
	vec4fp transpose3 = vec_mergel( _a, _c );
	vec4fp transpose4 = vec_mergel( _b, _d );
	a = vec_mergeh( transpose1, transpose2 );
	b = vec_mergel( transpose1, transpose2 );
	c = vec_mergeh( transpose3, transpose4 );
	d = vec_mergel( transpose3, transpose4 );
}

inline void vec_transpose (vec4fp _a, vec4fp _b, vec4fp _c, vec4fp _d,
									vec4fp& a, vec4fp& b, vec4fp& c)
{	// transpose 4 vector4 into 3 vector4 (not computing fourth output vector)
	vec4fp transpose1 = vec_mergeh( _a, _c );
	vec4fp transpose2 = vec_mergeh( _b, _d );
	vec4fp transpose3 = vec_mergel( _a, _c );
	vec4fp transpose4 = vec_mergel( _b, _d );
	a = vec_mergeh( transpose1, transpose2 );
	b = vec_mergel( transpose1, transpose2 );
	c = vec_mergeh( transpose3, transpose4 );
}

inline vec4fp vec_extract0 (vec4fp _a, vec4fp _b, vec4fp _c, vec4fp _d) { return(vec_mergeh( vec_mergeh( _a, _c ), vec_mergeh( _b, _d ) )); }
inline vec4fp vec_extract1 (vec4fp _a, vec4fp _b, vec4fp _c, vec4fp _d) { return(vec_mergel( vec_mergeh( _a, _c ), vec_mergeh( _b, _d ) )); }
inline vec4fp vec_extract2 (vec4fp _a, vec4fp _b, vec4fp _c, vec4fp _d) { return(vec_mergeh( vec_mergel( _a, _c ), vec_mergel( _b, _d ) )); }
inline vec4fp vec_extract3 (vec4fp _a, vec4fp _b, vec4fp _c, vec4fp _d) { return(vec_mergel( vec_mergel( _a, _c ), vec_mergel( _b, _d ) )); }

inline void vec_transpose (vec4uint _a, vec4uint _b, vec4uint _c, vec4uint _d,
									vec4uint& a, vec4uint& b, vec4uint& c, vec4uint& d)
{	// transpose 4 vector4 into 4 vector4
	vec4uint transpose1 = vec_mergeh( _a, _c );
	vec4uint transpose2 = vec_mergeh( _b, _d );
	vec4uint transpose3 = vec_mergel( _a, _c );
	vec4uint transpose4 = vec_mergel( _b, _d );
	a = vec_mergeh( transpose1, transpose2 );
	b = vec_mergel( transpose1, transpose2 );
	c = vec_mergeh( transpose3, transpose4 );
	d = vec_mergel( transpose3, transpose4 );
}

inline void vec_transpose (vec4uint _a, vec4uint _b, vec4uint _c, vec4uint _d,
									vec4uint& a, vec4uint& b, vec4uint& c)
{	// transpose 4 vector4 into 3 vector4 (not computing fourth output vector)
	vec4uint transpose1 = vec_mergeh( _a, _c );
	vec4uint transpose2 = vec_mergeh( _b, _d );
	vec4uint transpose3 = vec_mergel( _a, _c );
	vec4uint transpose4 = vec_mergel( _b, _d );
	a = vec_mergeh( transpose1, transpose2 );
	b = vec_mergel( transpose1, transpose2 );
	c = vec_mergeh( transpose3, transpose4 );
}

inline vec4uint vec_extract0 (vec4uint _a, vec4uint _b, vec4uint _c, vec4uint _d) { return(vec_mergeh( vec_mergeh( _a, _c ), vec_mergeh( _b, _d ) )); }
inline vec4uint vec_extract1 (vec4uint _a, vec4uint _b, vec4uint _c, vec4uint _d) { return(vec_mergel( vec_mergeh( _a, _c ), vec_mergeh( _b, _d ) )); }
inline vec4uint vec_extract2 (vec4uint _a, vec4uint _b, vec4uint _c, vec4uint _d) { return(vec_mergeh( vec_mergel( _a, _c ), vec_mergel( _b, _d ) )); }
inline vec4uint vec_extract3 (vec4uint _a, vec4uint _b, vec4uint _c, vec4uint _d) { return(vec_mergel( vec_mergel( _a, _c ), vec_mergel( _b, _d ) )); }

inline vec8ushort vec_interleavehh (vec8ushort src)
{
	vec8ushort temp = (vec8ushort)vec_sld(src, src, 8);				// from aaaabbbb create bbbbaaaa
	return((vec8ushort)vec_mergehh((vec4uint)src, (vec4uint)temp));	// merge high halves of aaaabbbb and bbbbaaaa to create abababab
}

inline vec8ushort vec_interleavelh (vec8ushort src)
{
	vec8ushort temp = (vec8ushort)vec_sld(src, src, 8);				// from aaaabbbb create bbbbaaaa
	return((vec8ushort)vec_mergelh((vec4uint)src, (vec4uint)temp));	// merge high halves of aaaabbbb and bbbbaaaa to create babababa
}

inline vec4fp vec_transform_point(const vec4fp v, const vec4fp xrow, const vec4fp yrow, const vec4fp zrow, const vec4fp wrow)
{
	vec4fp result = vec_madd(vec_splat_word(v,0), xrow, wrow);
	result = vec_madd(vec_splat_word(v,1), yrow, result);
	result = vec_madd(vec_splat_word(v,2), zrow, result);
	return(result);
}

inline vec4fp vec_transform_vector(const vec4fp v, const vec4fp xrow, const vec4fp yrow, const vec4fp zrow)
{
	vec4fp result = vec_mult(vec_splat_word(v,0), xrow);
	result = vec_madd(vec_splat_word(v,1), yrow, result);
	result = vec_madd(vec_splat_word(v,2), zrow, result);
	return(result);
}

inline vec4fp vec_interpolate (vec4fp d0, vec4fp d1, vec4fp blend, vec4fp blendc)
{
	return(vec_madd(blend, d0, vec_mult(blendc, d1)));
}

inline vec4fp vec_re (vec4fp v)
{
	vec4fp result;
	for(int i=0;i<4;++i)
		result.v[i] = 1.0f / v.v[i];
	return result;
}

inline vec4fp vec_recip (vec4fp v, vec4fp fpOne)
{
	return(vec_re(v));
}

inline vec4fp vec_rsqrte (vec4fp v)
{
	vec4fp result;
	for(int i=0;i<4;++i)
		result.v[i] = 1.0f / sqrtf(v.v[i]);
	return result;
}

inline vec4fp vec_fast_sqrt (vec4fp v)  // this is low precision
{
	vec4fp result;
	for(int i=0;i<4;++i)
		result.v[i] = sqrtf(v.v[i]);
	return result;
}

inline vec4fp vec_fast_recip_sqrt (vec4fp v)  // this is low precision
{
	return vec_rsqrte( v );
}

inline vec4fp vec_recip_sqrt (vec4fp v, vec4fp zero, vec4fp one, vec4fp half)
{	// newton rapheson refined recip estimate
	vec4fp sqrtRecipEst = vec_rsqrte( v );
	vec4fp recipEst = vec_madd( sqrtRecipEst, sqrtRecipEst, zero );
	vec4fp halfSqrtRecipEst = vec_madd( sqrtRecipEst, half, zero);
	vec4fp term1 = vec_nmsub( v, recipEst, one );
	return(vec_madd( term1, halfSqrtRecipEst, sqrtRecipEst ));
}

inline vec4fp vec_sqrt (vec4fp v, vec4fp zero, vec4fp one, vec4fp half)
{
	vec4fp result;
	for(int i=0;i<4;++i)
		result.v[i] = sqrtf(v.v[i]);
	return result;
}

//- debug printing support ------------------------------------------------------------------

	inline void print_vec4fp (const char* label, const vec4fp& v, uint32_t addr)
	{
		VEC_PRINTF("ppu:%s=[%8f,%8f,%8f,%8f]@%08x\n", label, v.v[0], v.v[1], v.v[2], v.v[3], addr);
	}

	inline void print_vec4fp (const char* label, const vec4fp& v) 
	{
		union { float f[4]; vec4fp v; } converter; converter.v = v;
		VEC_PRINTF("ppu:%s=[%8f,%8f,%8f,%8f]\n", label, v.v[0], v.v[1], v.v[2], v.v[3]);
	}

	inline void print_vec4uint (const char* label, const vec4uint& v, uint32_t addr)
	{
		VEC_PRINTF("ppu:%s=[%08x,%08x,%08x,%08x]@%08x\n", label, v.v[0], v.v[1], v.v[2], v.v[3], addr);
	}

	inline void print_vec4uint (const char* label, const vec4uint& v)
	{
		VEC_PRINTF("ppu:%s=[%08x,%08x,%08x,%08x]\n", label, v.v[0], v.v[1], v.v[2], v.v[3]);
	}

/////////////////////////////////////////// TBD... ///////////////////////////////////////////

//- half fp support -------------------------------------------------------------------------
// conversion of two 32-bit vector floats to one 16-bit "vector half"

// vec8ushort vec_pack_fp16 (vec4fp src0, vec4fp src1, vec4fp zero)
// vec4fp vec_unpack_fp16 (vec4uint src, vec4fp zero)
//#define vec_unpack_fp16l(src, zero) vec_unpack_fp16((vec4uint)vec_mergel((vec8ushort)zero, src), zero)
//#define vec_unpack_fp16h(src, zero) vec_unpack_fp16((vec4uint)vec_mergeh((vec8ushort)zero, src), zero)

//- 32-bit integer -> float support ---------------------------------------------------------

//#define vec_unpack_int4n(src, zero) vec_ctf(src, 31)
//#define vec_unpack_uint4n(src, zero) vec_ctf(vec_sr((vec4uint)(src), vec_immed_u32(1)), 31)

//- normalized short support ----------------------------------------------------------------
// Note that inputs are assumed to be in range [-1..+1]
// Precision is only guaranteed to 13 bits, although it is usually closer to 15 bits
// Conversion of two 32-bit vector floats to one 16-bit 8 element normalized short

// vec8short vec_pack_short8n (vec4fp src0, vec4fp src1, vec4fp zero)
// vec4fp vec_unpack_short8nl (vec8short src, vec4fp zero)
// vec4fp vec_unpack_short8nh (vec8short src, vec4fp zero)

//- normalized unsigned byte support --------------------------------------------------------
// Conversion of four 32-bit vector floats to one 8-bit 16 element normalized byte vector
// Note that inputs are assumed to be in range [0..+1]

// vec16byte vec_pack_byte16n (vec4fp src0, vec4fp src1, vec4fp src2, vec4fp src3, vec4fp zero)
// vec4fp internal_unpack_byte16n (vec16byte src, vec4fp zero)
//#define vec_unpack_byte16n(src, index, zero) internal_unpack_byte16n((vec16byte)vec_splat((vec4uint)src, index), zero)

//- uint <-> float4 conversions -------------------------------------------------------------
// Conversion between 32-bit floats and a packed formats

// vec4uint vec_pack_fp_s32 (vec4fp udata0, vec4fp udata1, vec4fp udata2, vec4fp udata3, vec4uint shifts, vec4uint masks, vec4fp scales, vec4fp offsets)
// void vec_unpack_fp_s32 (vec4uint pdata, vec4fp& udata0, vec4fp& udata1, vec4fp& udata2, vec4fp& udata3, vec4uint shifts, vec4uint rshifts, vec4uint masks, vec4fp scales, vec4fp offsets)
// void vec_pack_fp_compute_constants (vec4uint _shifts, vec4uint _bits, vec4fp _offsets, vec4uint& shifts, vec4uint& masks, vec4fp& scales, vec4fp& offsets)
// void vec_unpack_fp_compute_constants (vec4uint _shifts, vec4uint _bits, vec4fp _offsets, vec4uint& shifts, vec4uint& rshifts, vec4uint& masks, vec4fp& scales, vec4fp& offsets)
// void vec_fp_constants_norm_11_11_10 (vec4uint& shifts, vec4uint& bits, vec4fp& offsets)
// void vec_pack_fp_constants_norm_11_11_10 (vec4uint& shifts, vec4uint& masks, vec4fp& scales, vec4fp& offsets) { vec4uint _shifts, _bits; vec4fp _offsets; vec_fp_constants_norm_11_11_10(_shifts, _bits, _offsets); vec_pack_fp_compute_constants(_shifts, _bits, _offsets, shifts, masks, scales, offsets); }
// void vec_unpack_fp_constants_norm_11_11_10 (vec4uint& shifts, vec4uint& rshifts, vec4uint& masks, vec4fp& scales, vec4fp& offsets) { vec4uint _shifts, _bits; vec4fp _offsets; vec_fp_constants_norm_11_11_10(_shifts, _bits, _offsets); vec_unpack_fp_compute_constants(_shifts, _bits, _offsets, shifts, rshifts, masks, scales, offsets); }

#endif
