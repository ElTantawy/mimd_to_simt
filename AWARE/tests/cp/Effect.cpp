//  Copyright 2009 Electronic Arts, Inc. All rights reserved.
#include "Effect.h"
#include "Data.h"
#include "vectorutilities/vectorstreamutilities.h"

static bool kUseVectorizedIntegrator = false;

namespace ROPA
{
	Effect::Effect() : 
			mDT(0.0f),
			mProfilerMode(0),
			mNbParticles(0),
			mNbVertices(0),
			mNbVolumes(0),
			mNumConstraintSets(2),	// always at least 2 sets
			mDistanceCSIndex(0),	// distance is always index 0
			mDriverCSIndex(1),		// driver is always index 1
			mDstPositions(NULL),
			mDstStride(0),
			mEffectScale(1.0f),
			mBaseEffectScale(1.0f),
			mIntegratorFunc(NULL),
			mWriteBackFunc(NULL),
			mLockBuffersFunc(NULL)
	{
		mParticlesDebug = 0;

		for (int i = 0; i < MAX_CONSTRAINTSETS; ++i)
			mConstraintSolvers[i] = NULL;
	}

	Effect::~Effect ()
	{
		delete [] mParticlesDebug;
		delete [] mAtomicLocks;
	}

	void Effect::SetNbVertices(int n)
	{
		if (n != mNbVertices)
		{
			delete [] mReverseMapping;
			mReverseMapping = new uint16_t[n];
			mNbVertices = n;
		}
	}

	Effect& Effect::operator *= (float coef)
	{
		int i;
		for (i=0;i<mNumConstraintSets;i++)
		{
			int j,nbConstraintOctets = mConstraintsSets[i].GetNbConstraintOctets();

			ConstraintOctet* cArray = mArrays[i];

			for (j=0;j<nbConstraintOctets;j++)
				cArray[j] *= coef;
		}

		return *this;
	}

	bool Effect::Add(const Effect &rhs)
	{
		int i;
		for (i=0;i<mNumConstraintSets;i++)
		{
			int j,nbConstraintOctets = mConstraintsSets[i].GetNbConstraintOctets();

			if (nbConstraintOctets != rhs.mConstraintsSets[i].GetNbConstraintOctets())
				return false;

			ConstraintOctet* cArray1 = mArrays[i];
			ConstraintOctet* cArray2 = rhs.mArrays[i];

			for (j=0;j<nbConstraintOctets;j++)
				cArray1[j] += cArray2[j];
		}

		return true;
	}

	bool Effect::SubAndMult(const Effect &effect, float coef)
	{
		int i;
		for (i=0;i<mNumConstraintSets;i++)
		{
			int j,nbConstraintOctets = mConstraintsSets[i].GetNbConstraintOctets();

			if (nbConstraintOctets != effect.mConstraintsSets[i].GetNbConstraintOctets())
				return false;

			ConstraintOctet* cArray1 = mArrays[i];
			ConstraintOctet* cArray2 = effect.mArrays[i];

			for (j=0;j<nbConstraintOctets;j++)
				if (!cArray1[j].SubAndMult(cArray2[j],coef))
					return false;
		}
		return true;
	}

	void Effect::UpdateTuningParameters ()
	{
		mTuningParameters.mMaxDTRatio = 3.0f;
		mTuningParameters.mMinDTIntegration = 0.01f;
		mTuningParameters.mFixedDT = true;
		mTuningParameters.mVerticalSpeedDampening = 0.3075f;
		mTuningParameters.mCompression = 0.057f;
		mTuningParameters.mVertDistanceScale = 0.375f;
		mTuningParameters.mNormDistanceScale = 1.5f;
		mTuningParameters.mNbIterations = 1;
		mTuningParameters.mUseConstraintLengthOnNormals = true;
	}

	bool Effect::CheckParticlesIndices() const
	{
		int i;
		for (i=0;i<mNumConstraintSets;i++)
		{
			int j,nbConstraintOctets = mConstraintsSets[i].GetNbConstraintOctets();

			ConstraintOctet* cArray1 = mArrays[i];

			for (j=0;j<nbConstraintOctets;j++)
				if (!cArray1[j].CheckParticlesIndices(mNbParticles))
					return false;
		}
		return true;
	}

	void Effect::UpdateDistanceConstraints(const Vector4& scale,void* positions, size_t stride)
	{
		int iCS;
		for(iCS=0;iCS<mNumConstraintSets;iCS++)
		{
			mConstraintsSets[iCS].UpdateParticleDistanceConstraints(this,scale,positions,stride);
		}
	}

	static void EulerIntegrationOfDtRatio(float dtRatio, float &idealRatio, int &i)
	{
		float dtCumul	= 0.0f;
		float dtIter	= 1.0f;

		i=1;
		bool stop=false;
		while (!stop)
		{
			dtIter	*= idealRatio;
			dtCumul += dtIter;
			if (dtCumul > dtRatio)
				stop = true;
			else
				i++;
		}

		float delta,val;
		float x = idealRatio;
		float variation = idealRatio/2.0f;
		int j;
		int nIter = 0;
		static int maxIter = 50;
		do 
		{
			nIter++;
			val = x;
			for (j=1;j<i;j++)
			{
				val = val * x + x;
			}
			delta = dtRatio - val;
			if (delta < 0)
				x -= variation;
			else
				x += variation;
			variation /= 2.0f;
		}while (nIter < maxIter && fabsf(delta)> 1e-3);

		idealRatio = x;
	}

	void Effect::Load (const RopaBinaryHeader& header, const char* data)
	{
		mNbParticles = header.mNumParticles;
		mNbVertices = header.mNumVerts;
		mNbVolumes = header.mNumVolumes;
		mNumConstraintSets = header.mNumConstraintSets;
		mDistanceCSIndex = header.mDistanceCD;
		mDriverCSIndex = header.mDriverCD;
		mTcsIndex[0] = 0;
		mTcsIndex[1] = 0;
		
		mParticles = (Particle*)(data + header.mParticlesOffset);
		mVertexMapping = (int16_t*)(data + header.mVertexMapOffset);
		mMappedVertices = (uint32_t*)(data + header.mMappedVerticesOffset);
		mReverseMapping = (uint16_t*)(data + header.mReverseMapOffset);

		mParticlesDebug = new ParticleDebug[mNbParticles];
		
		mAtomicLocks = new int[mNbParticles]();

		mVolumesTransform = (VolumeTransform*)(data + header.mVolumesOffset);
		mDrivers = (PosNormPair*)(data + header.mDriversOffset);
		for (int cs = 0; cs < 4; ++cs)
		{
			mArrays[cs] = (ConstraintOctet*)(data + header.mConstraintOctetOffsets[cs]);
			mConstraintsSets[cs].SetSolverIdent((ESolverIdent)header.mSolvers[cs]);
			mConstraintsSets[cs].SetRefObjIndex(header.mObjects[cs]);
			mConstraintsSets[cs].SetConstraintsArray(mArrays[cs], header.mNumOctets[cs]);
		}
		
		mSystemPosition = header.mSystemPosition;
		mGravity = header.mGravity;
		mVertexPositionPackingOffset = header.mVertexPositionPackingOffset;
		mDT = header.mDT;
		
		mIntegratorState.mLastDT = header.mLastDT;
		mIntegratorState.mLastFullDT = header.mLastFullDT;
		mIntegratorState.mNbIntegrationSteps = 1;
	}

	EventID Effect::Update(EventID event, float dt, const Matrix44& systemPosition)
	{
		mSystemPosition = systemPosition;

		if (dt < ROPA::Epsilon)
			return(event);

		this->UpdateTuningParameters();

		mDeltaPos = mSystemPosition;

		mIntegratorState.mSpeed = 0.0f;

		mDT = dt;
 
		return this->DoUpdate(event, mIntegratorState);
	}

	EventID Effect::DoUpdate(EventID event, IntegratorState& iState) const
	{
		float dt = mDT;
		if (mTuningParameters.mFixedDT)
		{
			iState.mLastDTs[iState.mLastDTsIndex++] = dt;
			if (iState.mLastDTsIndex == IntegratorState::MAX_DTS)
				iState.mLastDTsIndex = 0;

			dt = 0.0f;
			for (int i=0;i<IntegratorState::MAX_DTS;i++)
				dt += iState.mLastDTs[i];

			dt /= IntegratorState::MAX_DTS;

			if (iState.mReset)
			{
				iState.mLastDT=dt;
				iState.mReset = false;
			}

			{
				if (mIntegratorFunc != NULL)
					event = (*mIntegratorFunc)(event, dt, mDeltaPos, mGravity, mTuningParameters, iState);
				else if (kUseVectorizedIntegrator)
				{
					this->LockBuffers();
					TCVIntegrate_Vectorized(dt, mDeltaPos);
					this->UnlockBuffers();
				}
				else
				{
					this->LockBuffers();
					TCVIntegrate(dt, mDeltaPos);
					this->UnlockBuffers();
				}

				iState.mLastDT = dt;
			}

			event = SolveConstraints(event, kUseVectorizedIntegrator?kVectorSolverMode:kScalarSolverMode);
		}
		else
		{
			if (dt > mTuningParameters.mMinDTIntegration)
			{
				static float maxDT = 1.0f;
				int nbInternalIter = static_cast<int>(dt / maxDT) + 1;
				float internalDt	= dt / nbInternalIter;

				for ( int k=0; k< nbInternalIter; k++)
				{
					float dtRatio = internalDt / iState.mLastDT;

					int		nbIterations;
					float	idealRatio = mTuningParameters.mMaxDTRatio;
					EulerIntegrationOfDtRatio(dtRatio,idealRatio, nbIterations);

					float dtIter = iState.mLastDT;
					for (int i=0;i<nbIterations; i++)
					{
						dtIter *= idealRatio;

						if (iState.mReset)
						{
							iState.mLastDT=dt;
							iState.mReset = false;
						}

						{
							if (mIntegratorFunc != NULL)
								event = (*mIntegratorFunc)(event, dtIter, mDeltaPos, mGravity, mTuningParameters, iState);
							else if (kUseVectorizedIntegrator)
							{
								this->LockBuffers();
								TCVIntegrate_Vectorized(dtIter, mDeltaPos);
								this->UnlockBuffers();
							}
							else
							{
								this->LockBuffers();
								TCVIntegrate(dtIter, mDeltaPos);
								this->UnlockBuffers();
							}

							iState.mLastDT = dt;
						}

						event = SolveConstraints(event, kUseVectorizedIntegrator?kVectorSolverMode:kScalarSolverMode);
					}

					iState.mLastFullDT = dt;
				}
			}
			else
				event = SolveConstraints(event, kUseVectorizedIntegrator?kVectorSolverMode:kScalarSolverMode);
		}

		event = this->WriteDestination(event);
		return event;
	}

	void Effect::TCVIntegrate(float dt, const Matrix44& accel) const
	{
		Particle *ptrParticle,*ptrParticleStoreBack;

		ptrParticle	= &mParticles[0];

		float	acc[3],curPos[3],curPrevPos[3],nextPos[3];

		acc[0] = mGravity.GetX() ;
		acc[1] = mGravity.GetY() ;
		acc[2] = mGravity.GetZ() ;


		int					pIdx= 0;

		acc[0]		*=	dt* dt;
		acc[1]		*=	dt* dt;
		acc[2]		*=	dt* dt;



		curPos[0]		= ptrParticle->mPos.GetX();
		curPos[1]		= ptrParticle->mPos.GetY();
		curPos[2]		= ptrParticle->mPos.GetZ();

		curPrevPos[0]	= ptrParticle->mPrevPos.GetX();
		curPrevPos[1]	= ptrParticle->mPrevPos.GetY();
		curPrevPos[2]	= ptrParticle->mPrevPos.GetZ();

		Vector4 prevPos;

		float verticalSpeedRatio = (1.0f - mTuningParameters.mVerticalSpeedDampening);

		for (;pIdx< mNbParticles; pIdx++)
		{

			// TIME CORRECTED 	 
			// xi+1 = xi + (xi - xi-1) * (dti / dti-1) + a * dti * dti
			//
			mParticlesDebug[pIdx].mColor = tRGBA(1.0f,1.0f,1.0f,1.0f);
			mParticlesDebug[pIdx].mLength = 0.0f;

			if (mVertexMapping[pIdx] >= 0 && ptrParticle->mPrevPos.GetW() > 0.0f)
			{
				nextPos[0]		=	curPos[0];
				nextPos[1]		=	curPos[1];
				nextPos[2]		=	curPos[2];

				ptrParticleStoreBack=	ptrParticle;

				nextPos[0]		-=	curPrevPos[0];
				nextPos[1]		-=	curPrevPos[1];
				nextPos[2]		-=	curPrevPos[2];

				nextPos[0]		*=  dt/mIntegratorState.mLastDT;
				nextPos[1]		*=  dt/mIntegratorState.mLastDT * verticalSpeedRatio;
				nextPos[2]		*=  dt/mIntegratorState.mLastDT;

				nextPos[0]		+=	acc[0];
				nextPos[1]		+=	acc[1];
				nextPos[2]		+=	acc[2];

				ptrParticle++;

				ptrParticleStoreBack->mPrevPos.SetX(ptrParticleStoreBack->mPos.GetX());
				ptrParticleStoreBack->mPrevPos.SetY(ptrParticleStoreBack->mPos.GetY());
				ptrParticleStoreBack->mPrevPos.SetZ(ptrParticleStoreBack->mPos.GetZ());

				nextPos[0]		+=	curPos[0];
				nextPos[1]		+=	curPos[1];
				nextPos[2]		+=	curPos[2];

				curPos[0]		= ptrParticle->mPos.GetX();
				curPos[1]		= ptrParticle->mPos.GetY();
				curPos[2]		= ptrParticle->mPos.GetZ();

				curPrevPos[0]	= ptrParticle->mPrevPos.GetX();
				curPrevPos[1]	= ptrParticle->mPrevPos.GetY();
				curPrevPos[2]	= ptrParticle->mPrevPos.GetZ();
				ptrParticleStoreBack->mPos.SetX(nextPos[0]);
				ptrParticleStoreBack->mPos.SetY(nextPos[1]);
				ptrParticleStoreBack->mPos.SetZ(nextPos[2]);
			}
			else
			{
				ptrParticle++;

				curPos[0]		= ptrParticle->mPos.GetX();
				curPos[1]		= ptrParticle->mPos.GetY();
				curPos[2]		= ptrParticle->mPos.GetZ();

				curPrevPos[0]	= ptrParticle->mPrevPos.GetX();
				curPrevPos[1]	= ptrParticle->mPrevPos.GetY();
				curPrevPos[2]	= ptrParticle->mPrevPos.GetZ();
			}
		}
	}

	void Effect::TCVIntegrate_Vectorized(float dt, const Matrix44& accel) const
	{
		this->TCVIntegrate(dt, accel);
	}


	EventID Effect::WriteDestination(EventID event) const
	{
		Particle *particles = mParticles;
		uint16_t *srcidx = mReverseMapping;
		int count = mNbVertices;
		
		//VEC_PRINTF("WriteDestination %d positions in format %d from 0x%08x[%d] to 0x%08x[%d] via 0x%08x\n", count, mVFormat, (uint32_t)particles, sizeof(Particle), (uint32_t)mDstPositions, mDstStride, (uint32_t)srcidx);

		// NOTE:  unfortunately there is (currently) no non-vectorized version of this function... there should be

		Matrix44 mat;	//= Inverse(mSystemPosition);
		SetIdentity(mat);
		
		if (mWriteBackFunc != NULL)
			event = (*mWriteBackFunc)(event, mat);
		else
		{
			this->LockBuffers();
			vec_transformconvertquadwordstream_srcindexed(count, srcidx, &mat, &particles->mPos, sizeof(Particle), mDstPositions, mDstStride, vec_put_float4_quartet());		
			this->UnlockBuffers();
		}
		return event;
	}

	EventID Effect::SolveConstraints(EventID event, ESolverMode mode) const
	{
		event = mConstraintsSets[mDistanceCSIndex].Solve(event, this, mDistanceCSIndex, mode, mConstraintSolvers[mDistanceCSIndex]);
		event = mConstraintsSets[mDriverCSIndex].Solve(event, this, mDriverCSIndex, mode, mConstraintSolvers[mDriverCSIndex]);
		for (int index = 0; index < mNumConstraintSets; ++index)
			if ((mConstraintsSets[index].GetSolverIdent() != kDriverSolver) && (mConstraintsSets[index].GetSolverIdent() != kParticleDistanceSolver))
				event = mConstraintsSets[index].Solve(event, this, index, mode, mConstraintSolvers[index]);
		return event;
	}

}
