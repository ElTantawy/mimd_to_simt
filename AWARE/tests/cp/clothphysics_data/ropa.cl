/*
 *  ropa.cl
 *  Ropa
 *
 *  Created by Andrew on 09-02-25.
 *  Copyright 2009 Electronic Arts, Inc. All rights reserved.
 *
 */
/*
#ifndef __OPENCL_VERSION__
	#define __kernel
	#define __global
	#define float2 float
#endif
*/
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics: enable

//#define ISCPU 1
#ifdef ISCPU
#include <stdio.h>
#define USE_TASK_VECTOR 1
#else
#define USE_TASK_VECTOR 0
#endif

typedef int cl_int;
typedef unsigned int cl_uint;

typedef struct Vector4
{
	float4 v;
} Vector4;

typedef struct Matrix44
{
	float16 m;
} Matrix44;

typedef struct Particle
{
	float4 mPos;
	float4 mPrevPos;
} Particle;

typedef struct ParticleDebug
{
	uchar4	mColor;
	float	mLength;
} ParticleDebug;

typedef struct VolumeTransform
{
	float16 mTransform;
	float16 mInvTransform;
} VolumeTransform;

typedef struct PosNormPair
{
	float4 mPosition;
	float4 mNormal;
} PosNormPair;

typedef struct ConstraintOctet
{
	short		mParticleIdx[8];
	short		mRefObjIdx[8];
	float		mMin[8];
	float		mMax[8];
} ConstraintOctet;

typedef struct ConstraintTuningParameters
{
	float	mMaxDTRatio;
	float	mMinDTIntegration;
	float	mFixedDT;
	float	mVerticalSpeedDampening;
	float	mCompression;
	float	mVertDistanceScale;
	float	mNormDistanceScale;
	int		mNbIterations;
	bool	mUseConstraintLengthOnNormals;
} ConstraintTuningParameters;

enum { MAX_DTS = 30 };

typedef struct IntegratorState
{
	float	mLastDTs[MAX_DTS];
	int		mLastDTsIndex;
	int		mNbIntegrationSteps;
	float	mLastDT;
	float	mLastFullDT;
	float	mSpeed;
	bool	mReset;
} IntegratorState;

#define MakeFloat8From4(HIGH, LOW) (float8)(HIGH.s0,HIGH.s1,HIGH.s2,HIGH.s3,LOW.s0,LOW.s1,LOW.s2,LOW.s3)

__attribute__((always_inline)) uchar4 tRGBA (float r, float g, float b, float a)
{
	return convert_uchar4((float4)(r,g,b,a) * 255.0f);
}

// transpose a 4x4 matrix
__attribute__((always_inline)) float16 transpose( float16 m ) 
{
	float16 t;
	t.even = m.lo;
	t.odd = m.hi;
	m.even = t.lo;
	m.odd = t.hi;
	return m;
}

// transform a point by a 4x4
__attribute__((always_inline)) float4 vec_transform_point( float16 m, float4 v)
{
	float4	result = mad(v.xxxx, m.s0123, m.scdef);
			result = mad(v.yyyy, m.s4567, result);
			result = mad(v.zzzz, m.s89ab, result);
	return result;
}

// transform a vector by a 4x4
__attribute__((always_inline)) float4 vec_transform_vector( float16 m, float4 v)
{
	float4	result = v.xxxx * m.s0123;
	result = mad(v.yyyy, m.s4567, result);
	result = mad(v.zzzz, m.s89ab, result);
	return result;
}

/*---------------------------------------------------------------------------------------------------------------*/
/*- Scalar implementations --------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------*/

__attribute__((always_inline)) void perform_integrator (int pIdx, float16 deltaPos, float4 gravity_impulse, float dt,
						__global Particle* particles, __global ParticleDebug* particlesDebug, __global short* indices, 
						__global IntegratorState *iState, __global ConstraintTuningParameters *ctp)
{
	float4	curPos, curPrevPos, nextPos;
	
	curPos = particles[pIdx].mPos;
	curPrevPos = particles[pIdx].mPrevPos;
	
	float verticalSpeedRatio = (1.0f - ctp->mVerticalSpeedDampening);
	
	// TIME CORRECTED VERLET 
	// xi+1 = xi + (xi - xi-1) * (dti / dti-1) + a * dti * dti
	//
	particlesDebug[pIdx].mColor = tRGBA(1.0f,1.0f,1.0f,1.0f);
	particlesDebug[pIdx].mLength = 0.0f;
		
	if ((indices[pIdx]) >= 0 && (curPrevPos.w > 0.0f))
	{
		nextPos = curPos;
		nextPos -= curPrevPos;
		nextPos *= dt/iState->mLastDT;
		nextPos.y *= verticalSpeedRatio;
		nextPos += gravity_impulse;
		
		particles[pIdx].mPrevPos = curPos;
		particles[pIdx].mPos = curPos + nextPos;
	}
}

__attribute__((always_inline)) void perform_writeback (int vertindex, float16 mat, __global ushort* indices, __global float4* vertices, 
						__global Particle* particles, __global ParticleDebug* particlesDebug)
{
	// output vertex draws from particle specified by its corresponding entry in indices
	ushort particleindex = indices[vertindex];
	
	// transform particle position by matrix
	float4 p = vec_transform_point(mat, particles[particleindex].mPos);
	
	// vertices are XYZc  where c is a 4 byte integer colour
	vertices[vertindex] = p;

	// TMA: commenting out since not supported by current NVIDIA driver (TODO: use atomics to create fence?)	
	//write_mem_fence(CLK_GLOBAL_MEM_FENCE);// double writing the w component, so have to ensure they are written in correct order
	
	__global uchar16 *colours = (__global uchar16 *)vertices;
	colours[vertindex].scdef = particlesDebug[particleindex].mColor;
}

__attribute__((always_inline)) void perform_distancesolve (int subIdx, float compressionTolerance, __global Particle *particles, __global ParticleDebug *particlesDebug, __global ConstraintOctet* octet)
{
	const float Epsilon = 1e-5f;
	
	__global Particle *p1,*p2;
	
	float	mag, factor;
	
	float4 p1p2;
	float4 repulse;
	
	int p1Idx = octet->mRefObjIdx[subIdx];
	int p2Idx = octet->mParticleIdx[subIdx];
	if ((p1Idx < 0) || (p2Idx < 0)) return;
	
	p1 = particles + p1Idx;
	p2 = particles + p2Idx;

	p1p2  = p2->mPos;
	p1p2 -= p1->mPos;

	p1p2.w = 0.0f;
	mag = fast_length(p1p2);

	// if the particles are onto each other, 
	// we do nothing, hoping that another constraint 
	// will pool them apart for the next iteration.
	if (mag > Epsilon)	
	{
		if (mag < (octet->mMin[subIdx] - compressionTolerance))
		{
			float invMass1 = p1->mPos.w;
			float invMass2 = p2->mPos.w;

			bool p1Locked = p1->mPrevPos.w < 0.0f;
			bool p2Locked = p2->mPrevPos.w < 0.0f;
			
			factor	= ((octet->mMin[subIdx] - compressionTolerance) - mag) / (mag * (invMass1 + invMass2));
			repulse	=	p1p2;
			repulse	*=	invMass1 * factor;
			p1p2	*=  invMass2 * factor;
			
			if (!p1Locked)
				p1->mPos -= repulse;
			else
				p1p2 += repulse;
			
			if (!p2Locked)
				p2->mPos += p1p2;
		}
		else if (mag > (octet->mMax[subIdx]) )
		{
			float invMass1 = p1->mPos.w;
			float invMass2 = p2->mPos.w;
			
			bool p1Locked = p1->mPrevPos.w < 0.0f;
			bool p2Locked = p2->mPrevPos.w < 0.0f;
			
			factor	= (mag - (octet->mMax[subIdx]) ) / (mag * (invMass1 + invMass2));
			
			repulse	=	p1p2;
			repulse	*=	invMass1 * factor;
			p1p2	*=  invMass2 * factor;
			
			if (!p1Locked)
				p1->mPos += repulse;
			else
				p1p2 -= repulse;
			
			if (!p2Locked)
				p2->mPos -= p1p2;
		}
	}
}

#ifdef TM
	__attribute__((noinline)) void __tbegin( void ) {
		// return __tbegin();
	}

	__attribute__((noinline)) void __tcommit( void ) {
		// return __tcommit();
	}

	__attribute__((always_inline)) void perform_distancesolve_tm (int subIdx, float compressionTolerance, __global Particle *particles, __global ParticleDebug *particlesDebug, __global ConstraintOctet* octet)
	{
		const float Epsilon = 1e-5f;

		__global Particle *p1,*p2;

		float	mag, factor;

		float4 p1p2;
		float4 repulse;

		int p1Idx = octet->mRefObjIdx[subIdx];
		int p2Idx = octet->mParticleIdx[subIdx];
		if ((p1Idx < 0) || (p2Idx < 0)) return;

		p1 = particles + p1Idx;
		p2 = particles + p2Idx;

		__tbegin();

		p1p2  = p2->mPos;
		p1p2 -= p1->mPos;

		p1p2.w = 0.0f;
		mag = fast_length(p1p2);

		// if the particles are onto each other,
		// we do nothing, hoping that another constraint
		// will pool them apart for the next iteration.
		if (mag > Epsilon)
		{
			if (mag < (octet->mMin[subIdx] - compressionTolerance))
			{
				float invMass1 = p1->mPos.w;
				float invMass2 = p2->mPos.w;

				bool p1Locked = p1->mPrevPos.w < 0.0f;
				bool p2Locked = p2->mPrevPos.w < 0.0f;

				factor	= ((octet->mMin[subIdx] - compressionTolerance) - mag) / (mag * (invMass1 + invMass2));
				repulse	=	p1p2;
				repulse	*=	invMass1 * factor;
				p1p2	*=  invMass2 * factor;

				if (!p1Locked)
					p1->mPos -= repulse;
				else
					p1p2 += repulse;

				if (!p2Locked)
					p2->mPos += p1p2;
			}
			else if (mag > (octet->mMax[subIdx]) )
			{
				float invMass1 = p1->mPos.w;
				float invMass2 = p2->mPos.w;

				bool p1Locked = p1->mPrevPos.w < 0.0f;
				bool p2Locked = p2->mPrevPos.w < 0.0f;

				factor	= (mag - (octet->mMax[subIdx]) ) / (mag * (invMass1 + invMass2));

				repulse	=	p1p2;
				repulse	*=	invMass1 * factor;
				p1p2	*=  invMass2 * factor;

				if (!p1Locked)
					p1->mPos += repulse;
				else
					p1p2 -= repulse;

				if (!p2Locked)
					p2->mPos -= p1p2;
			}
		}

		__tcommit();
	}
#endif

__attribute__((always_inline)) void perform_distancesolve_atomic (int subIdx, float compressionTolerance, __global Particle *particles, __global ParticleDebug *particlesDebug, __global ConstraintOctet* octet, __global int* atomicLocks)
{
   const float Epsilon = 1e-5f;

   __global Particle *p1,*p2;

   float mag, factor;

   float4 p1p2;
   float4 repulse;

   int p1Idx = octet->mRefObjIdx[subIdx];
   int p2Idx = octet->mParticleIdx[subIdx];
   if ((p1Idx < 0) || (p2Idx < 0)) return;
   if(p1Idx == p2Idx) return;

   p1 = particles + p1Idx;
   p2 = particles + p2Idx;


   volatile __global int* lock1;
   volatile __global int* lock2;
   if(p1Idx > p2Idx) {
      lock1 = atomicLocks + p1Idx;
      lock2 = atomicLocks + p2Idx;
   } else {
      lock1 = atomicLocks + p2Idx;
      lock2 = atomicLocks + p1Idx;
   }

   while(atom_cmpxchg(lock1, 0, 1)!=0)
   ;   
   while(atom_cmpxchg(lock2, 0, 1)!=0)
   ;   
   // Critical section
   p1p2  = p2->mPos;
   p1p2 -= p1->mPos;

   p1p2.w = 0.0f;
   mag = fast_length(p1p2);

   // if the particles are onto each other,
   // we do nothing, hoping that another constraint
   // will pool them apart for the next iteration.
   if (mag > Epsilon)
   {
   	if (mag < (octet->mMin[subIdx] - compressionTolerance))
        {
                  float invMass1 = p1->mPos.w;
                  float invMass2 = p2->mPos.w;

                  bool p1Locked = p1->mPrevPos.w < 0.0f;
                  bool p2Locked = p2->mPrevPos.w < 0.0f;

                  factor   = ((octet->mMin[subIdx] - compressionTolerance) - mag) / (mag * (invMass1 + invMass2));
                  repulse  =  p1p2;
                  repulse  *= invMass1 * factor;
                  p1p2  *=  invMass2 * factor;

                  if (!p1Locked)
                     p1->mPos -= repulse;
                  else
                     p1p2 += repulse;

                  if (!p2Locked)
                     p2->mPos += p1p2;
       }
       else if (mag > (octet->mMax[subIdx]) )
       {
                  float invMass1 = p1->mPos.w;
                  float invMass2 = p2->mPos.w;

                  bool p1Locked = p1->mPrevPos.w < 0.0f;
                  bool p2Locked = p2->mPrevPos.w < 0.0f;

                  factor   = (mag - (octet->mMax[subIdx]) ) / (mag * (invMass1 + invMass2));

                  repulse  =  p1p2;
                  repulse  *= invMass1 * factor;
                  p1p2  *=  invMass2 * factor;

                  if (!p1Locked)
                     p1->mPos += repulse;
                  else
                     p1p2 -= repulse;

                  if (!p2Locked)
                     p2->mPos -= p1p2;
      
      }
   }
    
   mem_fence(CLK_GLOBAL_MEM_FENCE);
   // Release locks
   atom_xchg(lock2, 0);
   atom_xchg(lock1, 0);
}

__attribute__((always_inline)) void perform_driversolve (int subIdx, float effectScale, float verticalDistanceScale, float normalDistanceScale, float invVDS, float invNDS,
						  __global Particle *particles, __global ParticleDebug *particlesDebug, __global ConstraintOctet* octet, __global PosNormPair *drivers)
{
	float4 p1pos, p1prevpos;
	float4 d1pos, d1norm;
	float4 VDSmult = (float4)(1.0f,verticalDistanceScale,1.0f,0.0f);
	float4 invVDSmult = (float4)(1.0f,invVDS,1.0f,0.0f);
	float4 pdColor;
	
	float4 pos,normal,tempNormal;
	
	float dotprod;
	float mag,max,useMin;
	
	VDSmult.x = 1.0f; VDSmult.y = verticalDistanceScale; VDSmult.z = 1.0f; VDSmult.w = 0.0f;
	invVDSmult.x = 1.0f; invVDSmult.y = invVDS; invVDSmult.z = 1.0f; invVDSmult.w = 0.0f;
	
	int pIdx = octet->mParticleIdx[subIdx];
	int dIdx = octet->mRefObjIdx[subIdx];
	if ((pIdx < 0) || (dIdx < 0)) return;
	
	p1pos = particles[pIdx].mPos;
	p1prevpos = particles[pIdx].mPrevPos;
	d1pos = drivers[dIdx].mPosition;
	d1norm = drivers[dIdx].mNormal;

	pdColor = convert_float4(particlesDebug[pIdx].mColor) * (1.0f/255.0f);

	if (p1prevpos.w >= 0.0f)
	{
		normal = d1norm;
		max = octet->mMin[subIdx];
		if (max > 0.0f)
			useMin = -1.0f;
		else
		{
			useMin = 1.0f;
			max = octet->mMax[subIdx];
		}

		normal *= useMin;
		
		particlesDebug[pIdx].mLength = useMin * max * normalDistanceScale;
		
		pos	= (d1pos - p1pos) * invVDSmult;
		
		tempNormal	 = normal * dot(normal, pos);
		pos			 = pos - tempNormal + tempNormal * invNDS;
		
		mag	= fast_length(pos);
		if (mag > max)
		{	// distance from driver too great
			pos	*= (mag - max) / mag;
			
			tempNormal	= normal * dot(normal, pos);
			pos			= (pos - tempNormal + tempNormal * normalDistanceScale) * VDSmult;
			
			p1pos += pos;
			pdColor -= (float4)(0.0f,0.5f,0.0f,0.0f);		// turn off green -- shows as purple
		}
		
		pos	= d1pos - p1pos;
		tempNormal = normal;
		
		pos.w = 0.0f;
		normal.w = 0.0f;
		dotprod = dot(tempNormal,pos);
		
		if (dotprod > 0.0f)
		{	// angle from normal too great
			p1pos += tempNormal * dotprod;
			pdColor -= (float4)(0.0f,0.0f,0.5f,0.0f);		// turn off blue -- shows as yellow
		}
		
		p1pos = d1pos + (p1pos - d1pos) * effectScale;
	}
	else
	{
		p1pos = (float4)(d1pos.x, d1pos.y, d1pos.z, p1pos.w);
		p1prevpos = (float4)(d1pos.x, d1pos.y, d1pos.z, p1prevpos.w);
	}

	particles[pIdx].mPos = p1pos;
	particles[pIdx].mPrevPos = p1prevpos;
	particlesDebug[pIdx].mColor = tRGBA(pdColor.x, pdColor.y, pdColor.z, pdColor.w);
}


__attribute__((always_inline)) void perform_vector_integrator (float4 vdt_ratio, float4 acc, cl_int numparticles, int pIdx,
								__global Particle *ptrParticle, __global ParticleDebug *ptrParticleDebug, __global uint *mapped)
{
	uint8 pdInit; pdInit.even = (uint4)~0; pdInit.odd = (uint4)0;
	*(__global uint8 *)(ptrParticleDebug) = pdInit;
	
	float16 curPos;
	curPos.s0123 = ptrParticle[0].mPos;
	curPos.s4567 = ptrParticle[1].mPos;
	curPos.s89ab = ptrParticle[2].mPos;
	curPos.scdef = ptrParticle[3].mPos;
	
	float16 curPrevPos;
	curPrevPos.s0123 = ptrParticle[0].mPrevPos;
	curPrevPos.s4567 = ptrParticle[1].mPrevPos;
	curPrevPos.s89ab = ptrParticle[2].mPrevPos;
	curPrevPos.scdef = ptrParticle[3].mPrevPos;
	
	curPos = transpose(curPos);
	curPrevPos = transpose(curPrevPos);
	
//	uint4 shiftmap = (uint4)(0,1,2,3) + (uint4)(pIdx&31);
//	uint4 basemask = ((mapped[pIdx>>5] >> shiftmap) & 1) - 1;			// each 32-bit uint is 8 sets of 4 bits, isolate 4 successive bits and convert to masks
//	uint4 basemask = (((uint4) (mapped[pIdx>>5] >> (pIdx&31)) & (uint4)(1,2,4,8) ) == 0;		// suggested alternative to avoid component-wise shift values
	int4 basemask = (int4)(0,0,0,0);
	int4 mask = ( (int4)isgreater(curPrevPos.scdef, (float4)0.0f) & ~basemask );
	
	// TIME CORRECTED VERLET 
	// xi+1 = xi + (xi - xi-1) * (dti / dti-1) + a * dti * dti
	float4 next_x = mad((curPos.s0123 - curPrevPos.s0123), vdt_ratio, (curPos.s0123 + acc.x));
	float4 next_y = mad((curPos.s4567 - curPrevPos.s4567), vdt_ratio, (curPos.s4567 + acc.y));
	float4 next_z = mad((curPos.s89ab - curPrevPos.s89ab), vdt_ratio, (curPos.s89ab + acc.z));
	
	curPrevPos.s0123 = select(curPrevPos.s0123, curPos.s0123, mask);
	curPrevPos.s4567 = select(curPrevPos.s4567, curPos.s4567, mask);
	curPrevPos.s89ab = select(curPrevPos.s89ab, curPos.s89ab, mask);
	
	curPos.s0123 = select(curPos.s0123, next_x, mask);
	curPos.s4567 = select(curPos.s4567, next_y, mask);
	curPos.s89ab = select(curPos.s89ab, next_z, mask);
	
	curPos = transpose(curPos);
	curPrevPos = transpose(curPrevPos);
	
	ptrParticle[0].mPos = curPos.s0123;
	ptrParticle[1].mPos = curPos.s4567;
	ptrParticle[2].mPos = curPos.s89ab;
	ptrParticle[3].mPos = curPos.scdef;
	
	ptrParticle[0].mPrevPos = curPrevPos.s0123;
	ptrParticle[1].mPrevPos = curPrevPos.s4567;
	ptrParticle[2].mPrevPos = curPrevPos.s89ab;
	ptrParticle[3].mPrevPos = curPrevPos.scdef;
}

__attribute__((always_inline)) void perform_vector_writeback (uint numverts, uint vertindex, float16 mat, __global ushort* indices, __global Particle* particles, __global ParticleDebug* particlesDebug, __global float4* vertices)
{
	// don't have a "real" 4-way implementation of this so just call the scalar one for times
	if (numverts >= 4)
	{
		ushort particleindex0 = indices[vertindex+0];
		ushort particleindex1 = indices[vertindex+1];
		ushort particleindex2 = indices[vertindex+2];
		ushort particleindex3 = indices[vertindex+3];
		
		float4 p0 = vec_transform_point(mat, particles[particleindex0].mPos);
		float4 p1 = vec_transform_point(mat, particles[particleindex1].mPos);
		float4 p2 = vec_transform_point(mat, particles[particleindex2].mPos);
		float4 p3 = vec_transform_point(mat, particles[particleindex3].mPos);
		
		vertices[vertindex+0] = p0;
		vertices[vertindex+1] = p1;
		vertices[vertindex+2] = p2;
		vertices[vertindex+3] = p3;
		
		// TMA: commenting out since not supported by current NVIDIA driver (TODO: use atomics to create fence?)	
		//write_mem_fence(CLK_GLOBAL_MEM_FENCE);	// double writing the w component, so have to ensure they are written in correct order
		
		__global uchar16 *colours = (__global uchar16 *)vertices;
		colours[vertindex+0].scdef = particlesDebug[particleindex0].mColor;
		colours[vertindex+1].scdef = particlesDebug[particleindex1].mColor;
		colours[vertindex+2].scdef = particlesDebug[particleindex2].mColor;
		colours[vertindex+3].scdef = particlesDebug[particleindex3].mColor;
	}
	else
	{
		switch (numverts)
		{
			default: break;
			case 3: perform_writeback(vertindex+2, mat, indices, vertices, particles, particlesDebug);
			case 2: perform_writeback(vertindex+1, mat, indices, vertices, particles, particlesDebug);
			case 1: perform_writeback(vertindex+0, mat, indices, vertices, particles, particlesDebug);
		}
	}
}

__attribute__((always_inline)) void perform_vector_distancesolver (float compressionTolerance, __global ConstraintOctet* constraints, __global Particle* particles, __global ParticleDebug* particlesDebug)
{
	const float8 Epsilon = 1e-5f, v0 = 0.0f, v1 = 1.0f;
	
	// get min & max values
	float8 cmin = *(__global float8*)constraints->mMin;
	float8 cmax = *(__global float8*)constraints->mMax;
	
	// get ref object indices
	short8 index1 = *(__global short8*)constraints->mRefObjIdx;
	short8 index2 = *(__global short8*)constraints->mParticleIdx;
	
	// load particle positions specified by ref objects
	float16 p11pos, p12pos, p21pos, p22pos;		// first digit is set 1 or set 2, second digit is particle 1 or particle 2
	
	p11pos.s0123 = particles[index1.s0].mPos;
	p11pos.s4567 = particles[index1.s1].mPos;
	p11pos.s89ab = particles[index1.s2].mPos;
	p11pos.scdef = particles[index1.s3].mPos;
	
	p21pos.s0123 = particles[index1.s4].mPos;
	p21pos.s4567 = particles[index1.s5].mPos;
	p21pos.s89ab = particles[index1.s6].mPos;
	p21pos.scdef = particles[index1.s7].mPos;
	
	p12pos.s0123 = particles[index2.s0].mPos;
	p12pos.s4567 = particles[index2.s1].mPos;
	p12pos.s89ab = particles[index2.s2].mPos;
	p12pos.scdef = particles[index2.s3].mPos;
	
	p22pos.s0123 = particles[index2.s4].mPos;
	p22pos.s4567 = particles[index2.s5].mPos;
	p22pos.s89ab = particles[index2.s6].mPos;
	p22pos.scdef = particles[index2.s7].mPos;
	
	float8 p1locked = (float8)(particles[index1.s0].mPrevPos.w, particles[index1.s1].mPrevPos.w, particles[index1.s2].mPrevPos.w, particles[index1.s3].mPrevPos.w, 
							   particles[index1.s4].mPrevPos.w, particles[index1.s5].mPrevPos.w, particles[index1.s6].mPrevPos.w, particles[index1.s7].mPrevPos.w);
	
	float8 p2locked = (float8)(particles[index2.s0].mPrevPos.w, particles[index2.s1].mPrevPos.w, particles[index2.s2].mPrevPos.w, particles[index2.s3].mPrevPos.w,
							   particles[index2.s4].mPrevPos.w, particles[index2.s5].mPrevPos.w, particles[index2.s6].mPrevPos.w, particles[index2.s7].mPrevPos.w);
	
	p11pos = transpose(p11pos);
	p21pos = transpose(p21pos);
	p12pos = transpose(p12pos);
	p22pos = transpose(p22pos);
	
	float8 p1x = MakeFloat8From4(p11pos.s0123, p21pos.s0123);
	float8 p1y = MakeFloat8From4(p11pos.s4567, p21pos.s4567);
	float8 p1z = MakeFloat8From4(p11pos.s89ab, p21pos.s89ab);
	float8 p1invmass = MakeFloat8From4(p11pos.scdef, p21pos.scdef);
	
	float8 p2x = MakeFloat8From4(p12pos.s0123, p22pos.s0123);
	float8 p2y = MakeFloat8From4(p12pos.s4567, p22pos.s4567);
	float8 p2z = MakeFloat8From4(p12pos.s89ab, p22pos.s89ab);
	float8 p2invmass = MakeFloat8From4(p12pos.scdef, p22pos.scdef);
	
	// delta from p1 to p2
	float8 p1p2_x = p2x - p1x;
	float8 p1p2_y = p2y - p1y;
	float8 p1p2_z = p2z - p1z;
	
	// magnitude of delta = sqrt(x*x+y*y+z*z)
	float8 mag = sqrt(p1p2_x * p1p2_x + p1p2_y * p1p2_y + p1p2_z * p1p2_z);
	float8 massOverMag = v1 / (mag * (p1invmass + p2invmass));
	float8 minLessTolerance = cmin - compressionTolerance;
	
	int8 p1NotLocked = isgreaterequal(p1locked, v0);	// if the points are nearly coincident, lock them both to suppress this calculation
	int8 p2NotLocked = isgreaterequal(p2locked, v0);
	
	int8 lessThanMin = isless(mag, minLessTolerance);
	int8 moreThanMax = isgreater(mag, cmax);
	
	float8 minFactor = (minLessTolerance - mag) * massOverMag;		// note that this is negated so that original subtract can be replaced with an add below
	float8 maxFactor = (mag - cmax) * massOverMag;
	
	// choose min vs max vs zero branch
	float8 factor = select(select(v0, maxFactor, moreThanMax), minFactor, lessThanMin);
	float8 p2AdjMult = select(v1, -v1, lessThanMin);
	
	float8 factorOverP1Mass = (p1invmass * factor);
	float8 factorOverP2Mass = (p2invmass * factor);
	
	float8 p1Repulse_x = (p1p2_x * factorOverP1Mass);
	float8 p1Repulse_y = (p1p2_y * factorOverP1Mass);
	float8 p1Repulse_z = (p1p2_z * factorOverP1Mass);
	
	float8 p2Repulse_x = (p1p2_x * factorOverP2Mass);
	float8 p2Repulse_y = (p1p2_y * factorOverP2Mass);
	float8 p2Repulse_z = (p1p2_z * factorOverP2Mass);
	
	// note that p2's repulsor is reduced by p1's if p1 is locked, but the reduction is negated if lessThanMin because the repulse was negated above
	p2Repulse_x -= select(p1Repulse_x, v0, p1NotLocked) * p2AdjMult;
	p2Repulse_y -= select(p1Repulse_y, v0, p1NotLocked) * p2AdjMult;
	p2Repulse_z -= select(p1Repulse_z, v0, p1NotLocked) * p2AdjMult;
	
	// don't move particles if they are almost coincident
	int8 notClose = isgreater(mag, Epsilon);
	p1NotLocked &= notClose;
	p2NotLocked &= notClose;
	
	// apply p1's repulsor if p1 is not locked
	p1x += select(v0, p1Repulse_x, p1NotLocked);
	p1y += select(v0, p1Repulse_y, p1NotLocked);
	p1z += select(v0, p1Repulse_z, p1NotLocked);
	
	// apply p2's repulsor if p2 is not locked
	p2x -= select(v0, p2Repulse_x, p2NotLocked);
	p2y -= select(v0, p2Repulse_y, p2NotLocked);
	p2z -= select(v0, p2Repulse_z, p2NotLocked);
	
	// back into matricies here
	p11pos.s0123 = p1x.lo;
	p21pos.s0123 = p1x.hi;
	p11pos.s4567 = p1y.lo;
	p21pos.s4567 = p1y.hi;
	p11pos.s89ab = p1z.lo;
	p21pos.s89ab = p1z.hi;
	
	p12pos.s0123 = p2x.lo;
	p22pos.s0123 = p2x.hi;
	p12pos.s4567 = p2y.lo;
	p22pos.s4567 = p2y.hi;
	p12pos.s89ab = p2z.lo;
	p22pos.s89ab = p2z.hi;
	
	// transform back to AoS
	p11pos = transpose(p11pos);
	p21pos = transpose(p21pos);
	p12pos = transpose(p12pos);
	p22pos = transpose(p22pos);
	
	// store back to indexed positions
	particles[index1.s0].mPos = p11pos.s0123;
	particles[index1.s1].mPos = p11pos.s4567;
	particles[index1.s2].mPos = p11pos.s89ab;
	particles[index1.s3].mPos = p11pos.scdef;
	
	particles[index1.s4].mPos = p21pos.s0123;
	particles[index1.s5].mPos = p21pos.s4567;
	particles[index1.s6].mPos = p21pos.s89ab;
	particles[index1.s7].mPos = p21pos.scdef;
	
	particles[index2.s0].mPos = p12pos.s0123;
	particles[index2.s1].mPos = p12pos.s4567;
	particles[index2.s2].mPos = p12pos.s89ab;
	particles[index2.s3].mPos = p12pos.scdef;
	
	particles[index2.s4].mPos = p22pos.s0123;
	particles[index2.s5].mPos = p22pos.s4567;
	particles[index2.s6].mPos = p22pos.s89ab;
	particles[index2.s7].mPos = p22pos.scdef;
}

__attribute__((always_inline)) void perform_vector_driversolver (float vertDistanceScale, float normDistanceScale, float invVertDistanceScale, float invNormDistanceScale, float effectScale,
								  __global Particle *particles, __global ParticleDebug *particlesDebug, __global ConstraintOctet *constraints, __global PosNormPair *drivers)
{
	const float8 Epsilon = 1e-5f, v0 = 0.0f, v1 = 1.0f;
	float8 vScale = effectScale;
	float8 vds = vertDistanceScale;
	float8 nds = normDistanceScale;
	float8 invVDS = invVertDistanceScale;
	float8 invNDS = invNormDistanceScale;
	
	// get min & max values
	float8 cmin = *(__global float8*)constraints->mMin;
	float8 cmax = *(__global float8*)constraints->mMax;
	
	int8 limitPositive = isgreater(cmin, v0);
	float8 limitUseMin = select(v1, -v1, limitPositive);
	float8 limit = select(cmax, cmin, limitPositive);
	
	float8 t0, t1, t2, t3, t4, t5, t6, t7;
	
	// load 8 particle positions, and 8 prev_w values into SOA format
	__global float8 *particlePtr = (__global float8 *)particles;
	t0 = particlePtr[0];
	t1 = particlePtr[1];
	t2 = particlePtr[2];
	t3 = particlePtr[3];
	t4 = particlePtr[4];
	t5 = particlePtr[5];
	t6 = particlePtr[6];
	t7 = particlePtr[7];
	
	float16 p1pos, p2pos;
	p1pos.s0123 = t0.s0123;
	p1pos.s4567 = t1.s0123;
	p1pos.s89ab = t2.s0123;
	p1pos.scdef = t3.s0123;
	p2pos.s0123 = t4.s0123;
	p2pos.s4567 = t5.s0123;
	p2pos.s89ab = t6.s0123;
	p2pos.scdef = t7.s0123;
	
	float8 pLocked = (float8)(t0.s7, t1.s7, t2.s7, t3.s7, t4.s7, t5.s7, t6.s7, t7.s7);
	
	p1pos = transpose(p1pos);
	p2pos = transpose(p2pos);
	
	float8 p1pos_x = MakeFloat8From4(p1pos.s0123, p2pos.s0123);
	float8 p1pos_y = MakeFloat8From4(p1pos.s4567, p2pos.s4567);
	float8 p1pos_z = MakeFloat8From4(p1pos.s89ab, p2pos.s89ab);
	
	// load 8 particle colours
	__global uint16 *debugPtr = (__global uint16 *)particlesDebug;
	uint8 colours = (*debugPtr).even;
	
	// load 8 driver positions and normals into SOA format
	__global float8 *driverPtr = (__global float8 *)drivers;
	t0 = driverPtr[0];
	t1 = driverPtr[1];
	t2 = driverPtr[2];
	t3 = driverPtr[3];
	t4 = driverPtr[4];
	t5 = driverPtr[5];
	t6 = driverPtr[6];
	t7 = driverPtr[7];
	
	float16 d1pos, d2pos;
	d1pos.s0123 = t0.s0123;
	d1pos.s4567 = t1.s0123;
	d1pos.s89ab = t2.s0123;
	d1pos.scdef = t3.s0123;
	d2pos.s0123 = t4.s0123;
	d2pos.s4567 = t5.s0123;
	d2pos.s89ab = t6.s0123;
	d2pos.scdef = t7.s0123;
	
	d1pos = transpose(d1pos);
	d2pos = transpose(d2pos);
	
	float16 d1nrm, d2nrm;
	d1nrm.s0123 = t0.s4567;
	d1nrm.s4567 = t1.s4567;
	d1nrm.s89ab = t2.s4567;
	d1nrm.scdef = t3.s4567;
	d2nrm.s0123 = t4.s4567;
	d2nrm.s4567 = t5.s4567;
	d2nrm.s89ab = t6.s4567;
	d2nrm.scdef = t7.s4567;
	
	d1nrm = transpose(d1nrm);
	d2nrm = transpose(d2nrm);
	
	float8 dpos_x = MakeFloat8From4(d1pos.s0123, d2pos.s0123);
	float8 dpos_y = MakeFloat8From4(d1pos.s4567, d2pos.s4567);
	float8 dpos_z = MakeFloat8From4(d1pos.s89ab, d2pos.s89ab);
	float8 dnrm_x = MakeFloat8From4(d1nrm.s0123, d2nrm.s0123);
	float8 dnrm_y = MakeFloat8From4(d1nrm.s4567, d2nrm.s4567);
	float8 dnrm_z = MakeFloat8From4(d1nrm.s89ab, d2nrm.s89ab);
	
	// perform driver calculation
	int8 p1NotLocked = isgreaterequal(pLocked, v0);
	
	float8 norm_x = dnrm_x * limitUseMin;
	float8 norm_y = dnrm_y * limitUseMin;
	float8 norm_z = dnrm_z * limitUseMin;
	
	float8 pos_x =  dpos_x - p1pos_x;
	float8 pos_y = (dpos_y - p1pos_y) * invVDS;
	float8 pos_z =  dpos_z - p1pos_z;
	
	float8 dotn = norm_x * pos_x + norm_y * pos_y + norm_z * pos_z;
	float8 tempNormal_x = norm_x * dotn;
	float8 tempNormal_y = norm_y * dotn;
	float8 tempNormal_z = norm_z * dotn;
	
	pos_x = (pos_x - tempNormal_x) * invNDS + tempNormal_x;
	pos_y = (pos_y - tempNormal_y) * invNDS + tempNormal_y;
	pos_z = (pos_z - tempNormal_z) * invNDS + tempNormal_z;
	
	float8 mag = sqrt(pos_x * pos_x + pos_y * pos_y + pos_z * pos_z);
	int8 overMax = isgreater(mag, limit);
	
	// pos<if over max> = pos * (mag - max)/mag
	float8 overpos_scale = (mag - limit) / mag;
	float8 overpos_x = pos_x * overpos_scale;
	float8 overpos_y = pos_y * overpos_scale;
	float8 overpos_z = pos_z * overpos_scale;
	
	float8 overdotn = norm_x * overpos_x + norm_y * overpos_y + norm_z * overpos_z;
	float8 overnorm_x = norm_x * overdotn;
	float8 overnorm_y = norm_y * overdotn;
	float8 overnorm_z = norm_z * overdotn;
	
	// pos = (pos - (norm*dot)) + (norm*dot)*normalDistanceScale		scale y as well
	overpos_x =  (overpos_x - overnorm_x) + overnorm_x * nds;
	overpos_y = ((overpos_y - overnorm_y) + overnorm_y * nds) * vds;
	overpos_z =	 (overpos_z - overnorm_z) + overnorm_z * nds;
	
	float8 newpos_x = p1pos_x + select(v0, overpos_x, overMax);
	float8 newpos_y = p1pos_y + select(v0, overpos_y, overMax);
	float8 newpos_z = p1pos_z + select(v0, overpos_z, overMax);
	uint8 g = select((uint8)0, (uint8)128, overMax);
	
	pos_x = (dpos_x - newpos_x);
	pos_y = (dpos_y - newpos_y);
	pos_z = (dpos_z - newpos_z);
	
	dotn = norm_x * pos_x + norm_y * pos_y + norm_z * pos_z;
	tempNormal_x = norm_x * dotn;
	tempNormal_y = norm_y * dotn;
	tempNormal_z = norm_z * dotn;
	int8 overZero = isgreater(dotn, v0);
	
	newpos_x += select(v0, tempNormal_x, overZero);
	newpos_y += select(v0, tempNormal_y, overZero);
	newpos_z += select(v0, tempNormal_z, overZero);
	uint8 b = select((uint8)0, (uint8)128, overZero);
	
	pos_x = (newpos_x - dpos_x) * vScale + dpos_x;
	pos_y = (newpos_y - dpos_y) * vScale + dpos_y;
	pos_z = (newpos_z - dpos_z) * vScale + dpos_z;
	
	p1pos_x = select(p1pos_x, pos_x, p1NotLocked);
	p1pos_y = select(p1pos_y, pos_y, p1NotLocked);
	p1pos_z = select(p1pos_z, pos_z, p1NotLocked);
	
	// put particle positions back where they came from
	p1pos.s0123 = p1pos_x.lo;
	p2pos.s0123 = p1pos_x.hi;
	p1pos.s4567 = p1pos_y.lo;
	p2pos.s4567 = p1pos_y.hi;
	p1pos.s89ab = p1pos_z.lo;
	p2pos.s89ab = p1pos_z.hi;
	
	p1pos = transpose(p1pos);
	p2pos = transpose(p2pos);
	
	particles[0].mPos = p1pos.s0123;
	particles[1].mPos = p1pos.s4567;
	particles[2].mPos = p1pos.s89ab;
	particles[3].mPos = p1pos.scdef;
	
	particles[4].mPos = p2pos.s0123;
	particles[5].mPos = p2pos.s4567;
	particles[6].mPos = p2pos.s89ab;
	particles[7].mPos = p2pos.scdef;
	
	uint8 newcolours  = colours - ((g << 8) | (b << 16));
	(*debugPtr).even = newcolours;
}
	
/*---------------------------------------------------------------------------------------------------------------*/
/*- Task scalar implementations----------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------*/

__kernel void task_integrator (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
							   __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles,
							   __global short *restrict indices, __global uint *restrict mapped, float16 deltaPos, float4 gravity, float dt)
{
	float4 gravity_impulse = gravity * dt * dt;
	for (int pIdx = 0; pIdx < numparticles; ++pIdx)
		perform_integrator(pIdx, deltaPos, gravity_impulse, dt, particles, particlesDebug, indices, iState, ctp);
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void task_writeback (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
							  __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
							  float16 mat, __global float4 *restrict vertices, __global ushort *restrict indices, cl_int numverts)
{
	for (int vertindex = 0; vertindex < numverts; ++vertindex)
		perform_writeback(vertindex, mat, indices, vertices, particles, particlesDebug);
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void task_distancesolver (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
								   __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
								   __global ConstraintOctet *restrict octets, cl_int numoctets)
{
	float comp = ctp->mCompression;

	for (int octet = 0; octet < numoctets; ++octet)
	{
		for (int subIdx = 0; subIdx < 8; ++subIdx)
			perform_distancesolve(subIdx, comp, particles, particlesDebug, &octets[octet]);

		mem_fence(CLK_GLOBAL_MEM_FENCE);
	}
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void task_driversolver (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
								 __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
								 __global ConstraintOctet *restrict octets, cl_int numoctets,
								 __global PosNormPair *restrict drivers)
{
	float vds = ctp->mVertDistanceScale;
	float nds = ctp->mNormDistanceScale;
	float invVDS = 1.0f / vds;
	float invNDS = 1.0f / nds;

	for (int octet = 0; octet < numoctets; ++octet)
	{
		for (int subIdx = 0; subIdx < 8; ++subIdx)
			perform_driversolve(subIdx, 1.0f, vds, nds, invVDS, invNDS, particles, particlesDebug, &octets[octet], drivers);
		
		mem_fence(CLK_GLOBAL_MEM_FENCE);
	}
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void task_spheresolver (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
								 __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
								 __global ConstraintOctet *restrict octets, cl_int numoctets,
								 __global VolumeTransform *restrict volumes)
{
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void task_tubesolver (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
							   __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
							   __global ConstraintOctet *restrict octets, cl_int numoctets,
							   __global VolumeTransform *restrict volumes)
{
}

/*---------------------------------------------------------------------------------------------------------------*/
/*- Scalar implementations---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------*/

__kernel void scalar_integrator (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
								 __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles,
								 __global short *restrict indices, __global uint *restrict mapped, float16 deltaPos, float4 gravity, float dt)
{
	uint numstrands = get_global_size(0);
	uint pCount = (uint)(numparticles + numstrands - 1) / numstrands;
	uint pIdx = (uint)get_global_id(0) * pCount;
	float4 acc = gravity * dt * dt;
	
	while (pCount-- > 0)
	{
		if (pIdx < numparticles)				// test to ensure we're not beyond the end of the array
		{
			perform_integrator(pIdx, deltaPos, acc, dt, particles, particlesDebug, indices, iState, ctp);
			pIdx += 1;
		}
		else
			return;
	}
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void scalar_writeback (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
								__global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
								float16 mat, __global float4 *restrict vertices, __global ushort *restrict indices, cl_int numverts)
{
	uint numstrands = get_global_size(0);
	uint vCount = (uint)(numverts + numstrands - 1) / numstrands;
	uint vertindex = (uint)get_global_id(0) * vCount;
	
	while (vCount-- > 0)
	{
		if (vertindex < numverts)
		{
			perform_writeback(vertindex, mat, indices, vertices, particles, particlesDebug);
			vertindex += 1;
		}
		else
			return;
	}
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void scalar_distancesolver (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
									 __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
									 __global ConstraintOctet *restrict octets, cl_int numoctets)
{
	float comp = ctp->mCompression;
	int numslices, firstslice, octetincrement, firstoctet;
	if (get_local_size(0) == 1)
	{	// the local group is only 1 work item, so it must process the entire octet
		numslices = 8;
		firstslice = 0;
		firstoctet = get_global_id(0);
		octetincrement = get_global_size(0);
	}
	else
	{	// a local group is multiple work items, each processing 1 slice of a group of octets
		numslices = 1;
		firstslice = get_global_id(0) & 7;
		firstoctet = get_global_id(0) >> 3;
		octetincrement = get_global_size(0) >> 3;
	}

	for (int octet = firstoctet; octet < numoctets; octet += octetincrement)
	{
		// compute constraint for slice of octet
		for (int slice = firstslice; slice < firstslice+numslices; ++slice)
			perform_distancesolve(slice, comp, particles, particlesDebug, &octets[octet]);
		
		barrier(CLK_GLOBAL_MEM_FENCE);	// ensure that all particles are written and all workitems are finished with this octet
	}
}


__kernel void scalar_distancesolver_atomic (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp,
                            __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles,
                            __global ConstraintOctet *restrict octets, cl_int numoctets, __global int *restrict atomicLocks)
{
   float comp = ctp->mCompression;
   int octet, slice;

   if(get_global_id(0) < numoctets * 8) {
      octet = get_global_id(0) >> 3;
      slice = get_global_id(0) & 7;
      perform_distancesolve_atomic(slice, comp, particles, particlesDebug, &octets[octet], atomicLocks);
   }

}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void scalar_driversolver (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
								   __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
								   __global ConstraintOctet *restrict octets, cl_int numoctets,
								   __global PosNormPair *restrict drivers)
{
	uint numstrands = get_global_size(0);
	uint cCount = (uint)(numoctets * 8 + numstrands - 1) / numstrands;
	uint cIdx = (uint)get_global_id(0) * cCount;
	
	float vds = ctp->mVertDistanceScale;
	float nds = ctp->mNormDistanceScale;
	float invVDS = 1.0f / vds;
	float invNDS = 1.0f / nds;

	while (cCount-- > 0)
	{
		if (cIdx < numoctets*8)
		{
			int slice = cIdx & 7;
			int octetindex = cIdx >> 3;
			perform_driversolve(slice, 1.0f, vds, nds, invVDS, invNDS, particles, particlesDebug, &octets[octetindex], drivers);
			cIdx += 1;
		}
		else
			return;
	}
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void scalar_spheresolver (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
								   __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
								   __global ConstraintOctet *restrict octets, cl_int numoctets,
								   __global VolumeTransform *restrict volumes)
{
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void scalar_tubesolver (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
								 __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
								 __global ConstraintOctet *restrict octets, cl_int numoctets,
								 __global VolumeTransform *restrict volumes)
{
}

/*---------------------------------------------------------------------------------------------------------------*/
/*- Vector implementations---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------*/

__kernel void vector_integrator (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
								 __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles,
								 __global short *restrict indices, __global uint *restrict mapped, float16 deltaPos, float4 gravity, float dt)
{
	float4 acc = gravity * dt * dt;
	float4 vdt_ratio = dt / iState->mLastDT;
	
	if (USE_TASK_VECTOR && (get_global_size(0) == 1))
	{
		for (int pIdx = 0; pIdx < numparticles; pIdx += 4)
			perform_vector_integrator(vdt_ratio, acc, numparticles, pIdx, particles+pIdx, particlesDebug+pIdx, mapped);
	}
	else 
	{
		uint numstrands = (get_global_size(0)*4);
		uint pCount = (uint)(numparticles + numstrands - 1) / numstrands;
		uint pIdx = (uint)get_global_id(0) * pCount * 4;
		
		while (pCount-- > 0)
		{
			if (pIdx < numparticles)				// test to ensure we're not beyond the end of the array
			{
				perform_vector_integrator(vdt_ratio, acc, numparticles, pIdx, particles+pIdx, particlesDebug+pIdx, mapped);
				pIdx += 4;
			}
			else
				return;
		}
	}
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void vector_writeback (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
								__global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
								float16 mat, __global float4 *restrict vertices, __global ushort *restrict indices, cl_int numverts)
{
	if (USE_TASK_VECTOR && (get_global_size(0) == 1))
	{
		for (int vIdx = 0; vIdx < numverts; vIdx += 4)
			perform_vector_writeback(numverts-vIdx, vIdx, mat, indices, particles, particlesDebug, vertices);
	}
	else
	{
		uint numstrands = (get_global_size(0)*4);
		uint vCount = (uint)(numverts + numstrands - 1) / numstrands;
		uint vertindex = (uint)get_global_id(0) * vCount * 4;

		while (vCount-- > 0)
		{
			if (vertindex < numverts)
			{
				perform_vector_writeback(numverts - vertindex, vertindex, mat, indices, particles, particlesDebug, vertices);
				vertindex += 4;
			}
			else
				return;
		}
	}
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void vector_distancesolver (__global IntegratorState *restrict iState, __global ConstraintTuningParameters *restrict ctp, 
									 __global Particle *restrict particles, __global ParticleDebug *restrict particlesDebug, cl_int numparticles, 
									 __global ConstraintOctet *restrict octets, cl_int numoctets)
{
	const float compressionTolerance = ctp->mCompression;

	if (USE_TASK_VECTOR && (get_global_size(0) == 1))
	{
		for (int octetindex = 0; octetindex < numoctets; ++octetindex)
			perform_vector_distancesolver(compressionTolerance, octets, particles, particlesDebug);
	}
	else
	{
		int octetincrement, firstoctet;
		if (get_local_size(0) == 1)
		{	// the local group is only 1 work item, so it must process the entire octet
			firstoctet = get_global_id(0);
			octetincrement = get_global_size(0);
		}
		else
		{	// a local group is multiple work items, each processing 1 slice of a group of octets
			firstoctet = get_global_id(0);
			octetincrement = get_global_size(0);
		}
		
		for (int octetindex = firstoctet; octetindex < numoctets; octetindex += octetincrement)
		{
			__global ConstraintOctet* constraints = octets+octetindex;
			perform_vector_distancesolver(compressionTolerance, constraints, particles, particlesDebug);		
			barrier(CLK_GLOBAL_MEM_FENCE);	// ensure that all particles are written and all workitems are finished with this octet
		}
	}
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void vector_driversolver (__global IntegratorState *iState, __global ConstraintTuningParameters *ctp, 
								   __global Particle *particles, __global ParticleDebug *particlesDebug, cl_int numparticles, 
								   __global ConstraintOctet *octets, cl_int numoctets,
								   __global PosNormPair *drivers)
{
	float vds = ctp->mVertDistanceScale;
	float nds = ctp->mNormDistanceScale;
	float invVDS = 1.0f / vds;
	float invNDS = 1.0f / nds;
	
	if (USE_TASK_VECTOR && (get_global_size(0) == 1))
	{
		for (int i = 0; i < numoctets; ++i)
			perform_vector_driversolver(vds, nds, invVDS, invNDS, 1.0f, particles+i*8, particlesDebug+i*8, octets+i, drivers+i*8);
	}
	else
	{
		uint numstrands = (get_global_size(0)*8);
		uint pCount = (uint)(numparticles + numstrands - 1) / numstrands;
		uint pIdx = (uint)get_global_id(0) * pCount * 8;
		
		while (pCount-- > 0)
		{
			if (pIdx < numparticles)				// test to ensure we're not beyond the end of the array
			{
				perform_vector_driversolver(vds, nds, invVDS, invNDS, 1.0f, particles+pIdx, particlesDebug+pIdx, octets+(pIdx>>3), drivers+pIdx);
				pIdx += 8;
			}
			else
				return;
		}
	}
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void vector_spheresolver (__global IntegratorState *iState, __global ConstraintTuningParameters *ctp, 
								   __global Particle *particles, __global ParticleDebug *particlesDebug, cl_int numparticles, 
								   __global ConstraintOctet *octets, cl_int numoctets,
								   __global VolumeTransform *volumes)
{
}

/*---------------------------------------------------------------------------------------------------------------*/

__kernel void vector_tubesolver (__global IntegratorState *iState, __global ConstraintTuningParameters *ctp, 
								 __global Particle *particles, __global ParticleDebug *particlesDebug, cl_int numparticles, 
								 __global ConstraintOctet *octets, cl_int numoctets,
								 __global VolumeTransform *volumes)
{
}
