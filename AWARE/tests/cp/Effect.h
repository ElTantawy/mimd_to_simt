//  Copyright 2009 Electronic Arts, Inc. All rights reserved.
#ifndef _EFFECT_H_
#define _EFFECT_H_

#include "Data.h"
#include "ConstraintsSets.h"

struct RopaBinaryHeader 
{
	float						mDT;
	float						mLastDT;
	float						mLastFullDT;
	float						mPad1;
	
	int							mNumParticles;
	int							mParticlesOffset;
	
	int							mNumDrivers;
	int							mDriversOffset;
	
	int							mNumVolumes;
	int							mVolumesOffset;
	
	int							mNumVertsMapped;
	int							mVertexMapOffset;
	
	int							mNumMappedVerts;
	int							mMappedVerticesOffset;
	
	int							mNumVerts;
	int							mReverseMapOffset;
	
	int							mNumTris;
	int							mVertexIndicesOffset;
	
	int							mNumConstraintSets;
	int							mOctetGroupSize;
	uint16_t					mDistanceCD;
	uint16_t					mDriverCD;
	int							mNumOctets[16];
	int							mConstraintOctetOffsets[16];
	int							mSolvers[16];
	int							mIterations[16];
	int							mObjects[16];
	
	Vector4						mGravity;
	Vector4						mVertexPositionPackingOffset;
	Matrix44					mSystemPosition;
	Matrix44					mDeltaPos;

	int							mNumFrames;
	int							mFramesOffset;
	int							mFrameTimesOffset;
};

namespace ROPA
{
	
	struct IntegratorState
	{
		enum { MAX_DTS = 30 };
		float	mLastDTs[MAX_DTS];
		int		mLastDTsIndex;
		int		mNbIntegrationSteps;
		float	mLastDT;
		float	mLastFullDT;
		float	mSpeed;
		bool	mReset;

		IntegratorState () : mLastDTsIndex(0), mNbIntegrationSteps(1), mLastDT(1.0f), mLastFullDT(1.0f/60.0f), mSpeed(0.0f), mReset(true) { for (int i=0;i<MAX_DTS;i++) mLastDTs[i] = 0.0f; }
	};
	
	struct IntegratorFunc { virtual EventID operator() (EventID event, float dt, const Matrix44& deltaPos, const Vector4& gravity, const ConstraintTuningParameters& ctp, IntegratorState& istate)=0; };
	struct WriteBackFunc { virtual EventID operator() (EventID event, const Matrix44& mat)=0; };
	struct LockBuffersFunc { virtual void LockBuffers()=0; virtual void UnlockBuffers()=0; };
	
	class Effect
	{
	public:
		Effect();
		~Effect();

		// API
		void			Load (const RopaBinaryHeader& header, const char* data);
		EventID			Update(EventID event, float dt, const Matrix44& systemPosition);
		void			Reset()	{ mIntegratorState.mReset=true; }

		// Set
		void SetNbIntegrationIteration(int n) { mIntegratorState.mNbIntegrationSteps = n; }
		void SetNbVertices(int n);
		void SetDestinationVertices(void* positions, size_t stride, const Vector4& offset) { mDstPositions = positions; mDstStride = stride; mVertexPositionPackingOffset = offset; }
		void SetSystemPosition(const Matrix44& systemPostion) {mSystemPosition = systemPostion;}
		void SetGravity (float g) { mGravity.y = g; }
		
		bool CheckParticlesIndices() const;

		void SetLockBuffersFunc (LockBuffersFunc* f) { mLockBuffersFunc = f; }
		void SetIntegratorFunc (IntegratorFunc* f) { mIntegratorFunc = f; }
		void SetWriteBackFunc (WriteBackFunc* f) { mWriteBackFunc = f; }
		void SetConstraintSolver (int index, ConstraintSolver* f) { mConstraintSolvers[index] = f; }
		
		// Get
		Particle*	GetParticles()		const	{return mParticles;}
		int			GetNbParticles()	const	{return mNbParticles;}
		int			GetNbVertices()		const	{return mNbVertices;}
		int			GetNbConstraintSets() const {return mNumConstraintSets;}
		float		GetGravity() const			{return mGravity.y;}
		
		int GetConstraintSetSolver(int index) const {return mConstraintsSets[index].GetSolverIdent();}
		void UpdateDistanceConstraints(const Vector4& scale, void* positions, size_t stride);

		// Effect Blending operators
		Effect& operator *= (float coef);
		bool Add(const Effect &rhs); 
		bool SubAndMult(const Effect &effect, float coef);

		VolumeTransform*		GetVolumesTransform(int index) const	{return mVolumesTransform+index;}
		int						GetNbVolumes() const					{return mNbVolumes;}

		PosNormPair*			GetDrivers() const						{return mDrivers;}
		int						GetDriverConstraintsIndex()				{return mDriverCSIndex;}
		int						GetDistanceConstraintsIndex()			{return mDistanceCSIndex;}
		ConstraintsSet*			GetDriverConstraints()					{return &mConstraintsSets[mDriverCSIndex];}
		ConstraintsSet*			GetDistanceConstraints()				{return &mConstraintsSets[mDistanceCSIndex];}
		ConstraintsSet*			GetConstraintsSet(int index)			{return &mConstraintsSets[index];}

		const int16_t* GetVertexMappingArray() const	{ return mVertexMapping; }
		void SetVertexMapping(int pIndex, int vIndex) 
		{
			xASSERT(mReverseMapping != NULL);
			mVertexMapping[pIndex] = vIndex;
			mReverseMapping[vIndex] = pIndex;
			mMappedVertices[pIndex>>5] |= 1 << (pIndex&31); 
		}

		const uint32_t* GetMappedVertexFlags() const { return mMappedVertices; }
		const uint16_t* GetReverseVertexMappingArray() const { return mReverseMapping; }
		const ConstraintTuningParameters& GetConstraintTuningParameters() const { return(mTuningParameters); }
		IntegratorState& GetIntegratorState() { return(mIntegratorState); }

		float GetSpeed() const {return mIntegratorState.mSpeed;}
		int ProfilerMode() const { return mProfilerMode; }

		EventID DoUpdate(EventID event, IntegratorState& iState) const;

		// debug support
		ParticleDebug* GetParticlesDebug() const	{return mParticlesDebug;}

		int* GetAtomicLocks() const { return mAtomicLocks; }

		void	SetScale(float s)				{mEffectScale=s;}
		float	GetScale()				const	{return mEffectScale*mBaseEffectScale;}

		void	SetBaseScale(float s)			{mBaseEffectScale=s;}
		float	GetBaseScale()			const	{return mBaseEffectScale;}

		EventID WriteDestination(EventID event) const;

		void LockBuffers () const { if (mLockBuffersFunc != NULL) mLockBuffersFunc->LockBuffers(); }
		void UnlockBuffers () const { if (mLockBuffersFunc != NULL) mLockBuffersFunc->UnlockBuffers(); }

		void UpdateTuningParameters ();

	private:
		//
		// Internals -- do NOT #ifdef any fields as this can cause a size/alignment mismatch between processors!
		//
		void TCVIntegrate (float dt, const Matrix44& accel) const;
		void TCVIntegrate_Vectorized (float dt, const Matrix44& accel) const;

		EventID SolveConstraints (EventID event, ESolverMode mode_t) const;

		IntegratorState				mIntegratorState;
		ConstraintTuningParameters	mTuningParameters;
		Vector4						mGravity;

		Matrix44	mDeltaPos;	// DoUpdate parameter
		Vector4		mVertexPositionPackingOffset;
		float		mDT;			// DoUpdate parameter

		int		mProfilerMode;

		int		mNbParticles;
		int		mNbVertices;
		int		mNbVolumes;
		int		mNumConstraintSets;
		int		mDistanceCSIndex;
		int		mDriverCSIndex;
		int		mTcsIndex[2];

		void*	mDstPositions;
		size_t	mDstStride;

		Particle*	mParticles;
		int16_t*	mVertexMapping;
		uint32_t*	mMappedVertices;	// bitflag array
		uint16_t*	mReverseMapping;

		int*        mAtomicLocks;

		ParticleDebug	*mParticlesDebug;	// allocated

		float mEffectScale;
		float mBaseEffectScale;

#define MAX_CONSTRAINTSETS	16
		VolumeTransform*	mVolumesTransform;
		PosNormPair*		mDrivers;
		ConstraintOctet*	mArrays[MAX_CONSTRAINTSETS];

		Matrix44	mSystemPosition;

		ConstraintsSet	mConstraintsSets[MAX_CONSTRAINTSETS];

		// revectoring implementations
		IntegratorFunc*		mIntegratorFunc;
		WriteBackFunc*		mWriteBackFunc;
		ConstraintSolver*	mConstraintSolvers[MAX_CONSTRAINTSETS];
		LockBuffersFunc*	mLockBuffersFunc;
	};
}

#endif // _EFFECT_H_
