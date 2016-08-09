//  Copyright 2009 Electronic Arts, Inc. All rights reserved.

#include "ConstraintsSets.h"
#include "Effect.h"

namespace ROPA
{
	void ConstraintsSet::UpdateParticleDistanceConstraints(const Effect* effect, const Vector4& scale, void* positions, size_t stride)
	{
		if (mSolverIdent != kParticleDistanceSolver)
			return;

		const short *mappingArray = effect->GetVertexMappingArray();
		unsigned char* srcbytes = (unsigned char*)positions;

		int iCO;
		int icst;
		int vertexIndex1,vertexIndex2;
		Vector4 pos1(0.0f,0.0f,0.0f,1.0f);
		Vector4 pos2(0.0f,0.0f,0.0f,1.0f);

			{
				for (iCO=0;iCO<mNbConstraintOctets;iCO++)
				{
					for (icst=0;icst<8;icst++)
					{
						vertexIndex1 = mappingArray[mArray[iCO].mParticleIdx[icst]];
						vertexIndex2 = mappingArray[mArray[iCO].mRefObjIdx[icst]];
						if (vertexIndex1 >= 0 && vertexIndex2 >= 0)
						{
							pos1 = *(Vector4*)(srcbytes + vertexIndex1 * stride);
							pos2 = *(Vector4*)(srcbytes + vertexIndex2 * stride);

							pos1 -=pos2;
							pos1.SetX(pos1.GetX()* scale.GetX());
							pos1.SetY(pos1.GetY()* scale.GetY());
							pos1.SetZ(pos1.GetZ()* scale.GetZ());
							mArray[iCO].mMax[icst] = Magnitude(pos1);
						}
					}
				}
			}
	}

	EventID ConstraintsSet::TestSolver (EventID event, const Effect* effect, int index, ESolverMode mode, ConstraintSolver* solver) const
	{
		return this->Solve(event, effect, index, kScalarSolverMode, solver);
	}
	
	EventID ConstraintsSet::Solve (EventID event, const Effect* effect, int index, ESolverMode mode, ConstraintSolver* cs) const
	{
		if (cs != NULL)
			return (*cs)(event, index, mNbConstraintOctets, mNbIterations, mRefObjIndex);
		else
		{
			switch (mode)
			{
				case kScalarSolverMode:
					effect->LockBuffers();
					switch (mSolverIdent) 
					{
						case kNullSolver:															break;	
						case kParticleDistanceSolver:	this->ParticleDistanceSolve_scalar(effect);	break;	
						case kDriverSolver:				this->DriverSolve_scalar(effect);			break;	
						case kSphereSolver:				this->SphereSolve_scalar(effect);			break;	
						case kTubeSolver:				this->TubeSolve_scalar(effect);				break;	
						case kCubeSolver:				this->CubeSolve_scalar(effect);				break;	
						case kPlaneSolver:				this->PlaneSolve_scalar(effect);			break; 
					}
					effect->UnlockBuffers();
					break;
				case kVectorSolverMode:
					effect->LockBuffers();
					switch (mSolverIdent) 
					{
						case kNullSolver:															break;	
						case kParticleDistanceSolver:	this->ParticleDistanceSolve_vector(effect);	break;	
						case kDriverSolver:				this->DriverSolve_vector(effect);			break;	
						case kSphereSolver:				this->SphereSolve_vector(effect);			break;	
						case kTubeSolver:				this->TubeSolve_vector(effect);				break;	
						case kCubeSolver:				this->CubeSolve_vector(effect);				break;	
						case kPlaneSolver:				this->PlaneSolve_vector(effect);			break; 
					}
					effect->UnlockBuffers();
					break;
				case kTestSolverMode:
					event = this->TestSolver(event, effect, index, (ESolverMode)mSolverIdent, cs);
					break;
			}
			return event;
		}
	}
	

	void ConstraintsSet::ParticleDistanceSolve_scalar(const Effect* effect) const
	{
		Particle	*particles = effect->GetParticles();
		Particle	*p1,*p2;

		ParticleDebug	*particlesDebug = effect->GetParticlesDebug();
		ParticleDebug	*p1d,*p2d;

		int		cIdx,cIter;
		float	mag, factor;

		Vector4 p1p2;
		Vector4 repulse;

		float	compressionTolerance	= effect->GetConstraintTuningParameters().mCompression;
		int		nbIterations			= effect->GetConstraintTuningParameters().mNbIterations;

		for (cIter=0;cIter<nbIterations;cIter++)
		{
			for (cIdx = 0; cIdx < mNbConstraintOctets; ++cIdx)
			{
				for (int subIdx = 0; subIdx < 8; ++subIdx)
				{
					int p1Idx = mArray[cIdx].mRefObjIdx[subIdx];
					int p2Idx = mArray[cIdx].mParticleIdx[subIdx];
					if ((p1Idx < 0) || (p2Idx < 0)) continue;
					p1 = particles + p1Idx;
					p2 = particles + p2Idx;

					p1d = particlesDebug + mArray[cIdx].mRefObjIdx[subIdx];
					p2d = particlesDebug + mArray[cIdx].mParticleIdx[subIdx];

					p1p2	=	p2->mPos;
					p1p2	-=	p1->mPos;

					p1p2.SetW(0.0);
					mag = Magnitude(p1p2);

					// if the particles are onto each other, 
					// we do nothing, hoping that another constraint 
					// will pool them apart for the next iteration.
					if (mag > ROPA::Epsilon)	
					{
						if (mag < (mArray[cIdx].mMin[subIdx] - compressionTolerance))
						{
							float invMass1 = p1->mPos.GetW() ;
							float invMass2 = p2->mPos.GetW() ;

							bool p1Locked = p1->mPrevPos.GetW() < 0.0f;
							bool p2Locked = p2->mPrevPos.GetW() < 0.0f;

							factor	= ((mArray[cIdx].mMin[subIdx] - compressionTolerance) - mag) / (mag * (invMass1 + invMass2));
							repulse	=	p1p2;
							repulse	*=	invMass1 * factor;
							p1p2	*=  invMass2 * factor;

							if (!p1Locked)
								p1->mPos-=  repulse;
							else
								p1p2 += repulse;

							if (!p2Locked)
								p2->mPos+=	p1p2;
						}
						else if (mag > (mArray[cIdx].mMax[subIdx]) )
						{
							float invMass1 = p1->mPos.GetW() ;
							float invMass2 = p2->mPos.GetW() ;

							bool p1Locked = p1->mPrevPos.GetW() < 0.0f;
							bool p2Locked = p2->mPrevPos.GetW() < 0.0f;

							factor	= (mag - (mArray[cIdx].mMax[subIdx]) ) / (mag * (invMass1 + invMass2));
							repulse	=	p1p2;
							repulse	*=	invMass1 * factor;
							p1p2	*=  invMass2 * factor;

							if (!p1Locked)
								p1->mPos+=  repulse;
							else
								p1p2 -= repulse;


							if (!p2Locked)
								p2->mPos-=	p1p2;
						}
					}
				}
			}
		}
	}

	void ConstraintsSet::ParticleDistanceSolve_vector(const Effect* effect) const
	{
		this->ParticleDistanceSolve_scalar(effect);
	}

	void ConstraintsSet::DriverSolve_scalar(const Effect* effect) const
	{
		Particle	*particles = effect->GetParticles();
		Particle	*p1;

		ParticleDebug	*particlesDebug = effect->GetParticlesDebug();
		ParticleDebug	*p1d;

		PosNormPair		*drivers	= effect->GetDrivers();
		PosNormPair		*d1;

		int			cIdx,cIter;

		Vector4 pos,prevpos,speed;
		Vector4 normal,tempNormal;
 
		float dot;
		float mag,max,useMin;

		float effectScale = effect->GetScale();

		float verticalDistanceScale = effect->GetConstraintTuningParameters().mVertDistanceScale;
		float normalDistanceScale	= effect->GetConstraintTuningParameters().mNormDistanceScale;

		for (cIter=0;cIter<mNbIterations;cIter++)
		{
			for (cIdx = 0; cIdx < mNbConstraintOctets; ++cIdx)
			{
				for (int subIdx = 0; subIdx < 8; ++subIdx)
				{
					int pIdx	= mArray[cIdx].mParticleIdx[subIdx];
					int dIdx	= mArray[cIdx].mRefObjIdx[subIdx];
					if ((pIdx < 0) || (dIdx < 0)) continue;
					p1	= particles	+ pIdx;
					d1	= drivers + dIdx;

					if (p1->mPrevPos.GetW() >= 0.0f)
					{
						normal	=	d1->mNormal;
						max = mArray[cIdx].mMin[subIdx];
						if ( max > 0.0f)
							useMin = -1.0f;
						else
						{
							useMin = 1.0f;
							max = mArray[cIdx].mMax[subIdx];
						}

						normal *= useMin;

						p1d = particlesDebug + mArray[cIdx].mParticleIdx[subIdx];
						p1d->mLength = useMin * max * effect->GetConstraintTuningParameters().mNormDistanceScale;

						tempNormal = normal;

						pos	=	d1->mPosition;
						pos	-=	p1->mPos;
						pos.SetW(0.0f);
						pos.SetY(pos.GetY() /  verticalDistanceScale);

						dot		=	Dot(normal, pos);
						tempNormal	*=	dot;
						pos			-=	tempNormal;

						tempNormal	/= normalDistanceScale;
						pos			+= tempNormal;

						mag	= Magnitude(pos);

//						p1d->mColor = tRGBA(max, 1.0f, 1.0f, 1.0f);

						if (mag > max )
						{	// distance from driver too great
							pos		*= (mag - max) / mag;

							tempNormal	= normal;
							dot		= Dot(normal, pos);
							tempNormal	*= dot;
							pos			-= tempNormal;

							tempNormal	*= normalDistanceScale;
							pos			+= tempNormal;

							pos.SetY(pos.GetY() * verticalDistanceScale);
							p1->mPos += pos;

							p1d->mColor -= tRGBA(0.0f,0.5f,0.0f,0.0f);		// turn off green -- shows as purple
						}

						pos		= d1->mPosition;
						tempNormal	= normal;

						pos		-=	p1->mPos;
						pos.SetW(0.0f);
						normal.SetW(0.0f);
						dot			= Dot(tempNormal,pos);

						if (dot > 0.0f)
						{	// angle from normal too great
							tempNormal	*= dot;
							p1->mPos	+= tempNormal;

							p1d->mColor -= tRGBA(0.0f,0.0f,0.5f,0.0f);		// turn off blue -- shows as yellow
						}
						pos = p1->mPos;
						pos -= d1->mPosition;
						pos *= effectScale;
						p1->mPos = d1->mPosition;
						p1->mPos += pos;
					}
					else
					{
						p1->Init(d1->mPosition);
					}
				} // subindices
			} // octets
		} // iterations
	}

	void ConstraintsSet::DriverSolve_vector(const Effect* effect) const
	{
		this->DriverSolve_scalar(effect);
	}

	void ConstraintsSet::TubeSolve_scalar (const Effect *effect) const
	{
		Particle	*particles = effect->GetParticles();
		Particle	*p1;

		VolumeTransform	*v1 = effect->GetVolumesTransform(mRefObjIndex);

		int			cIdx,cIter;
		Vector4		repulse;

		float baseRadius	= 0.5f;
		float baseRadiusSqr = 0.5f * 0.5f;
		float baseHeight	= 1.0f;

		for (cIter=0;cIter<mNbIterations;cIter++)
		{
			for (cIdx = 0; cIdx < mNbConstraintOctets; ++cIdx)
			{
				for (int subIdx = 0; subIdx < 8; ++subIdx)
				{
					int pIdx = mArray[cIdx].mParticleIdx[subIdx];
					if (pIdx < 0) continue;
					p1 = particles	+ pIdx;

					if (p1->mPrevPos.GetW() >= 0.0f)
					{
						Vector4 localPos = TransformPoint(*reinterpret_cast<Vector4*>(&p1->mPos), v1->mInvTransform);

						// Unit Tube specific collision code
						if (localPos.GetX() < baseHeight && localPos.GetX() >0.0f)
						{
							float mag = localPos.GetY() * localPos.GetY() + localPos.GetZ() * localPos.GetZ();

							if (mag < baseRadiusSqr)
							{
								//was:
								// mag = baseRadius - Sqrt(mag);
								// mag /= (baseRadius - mag);
								//algebraic reduction of scalar math yields:
								//		mag = (baseRadius - Sqrt(mag)) / (baseRadius - (baseRadius - Sqrt(mag)))
								//		mag = (baseRadius - Sqrt(mag)) / (baseRadius - baseRadius + Sqrt(mag))
								//		mag = (baseRadius - Sqrt(mag)) / Sqrt(mag)
								//		mag = baseRadius / Sqrt(mag) - Sqrt(mag) / Sqrt(mag)
								//		mag = baseRadius / Sqrt(mag) - 1.0
								mag = (baseRadius / sqrtf(mag)) - 1.0f;

								repulse.Set(0.0f, localPos.GetY() * mag , localPos.GetZ() * mag);
								repulse = TransformVector(repulse, v1->mTransform);

								float w = p1->mPos.GetW();
								p1->mPos += *reinterpret_cast<Vector4*>(&repulse);
								p1->mPos.SetW(w);

			/*						repulse *= 0.5f;

								w = p1->mPrevPos.GetW();
								p1->mPrevPos += *reinterpret_cast<Vector4*>(&repulse);
								p1->mPrevPos.SetW(w);
			*/					
							}
							// End Collision code
						}
					}
				}
			}
		}
	}


	void ConstraintsSet::TubeSolve_vector (const Effect *effect) const
	{
		this->TubeSolve_scalar(effect);
	}

	void ConstraintsSet::SphereSolve_scalar (const Effect *effect) const
	{
		Particle	*particles = effect->GetParticles();
		Particle	*p1;

		VolumeTransform	*v1 = effect->GetVolumesTransform(mRefObjIndex);

		int			cIdx,cIter;

		Vector4 repulse;

		float baseRadius	= 0.5f;
		float baseRadiusSqr = 0.5f * 0.5f;

		for (cIter=0;cIter<mNbIterations;cIter++)
		{
			for (cIdx = 0; cIdx < mNbConstraintOctets; ++cIdx)
			{
				for (int subIdx = 0; subIdx < 8; ++subIdx)
				{
					int pIdx = mArray[cIdx].mParticleIdx[subIdx];
					if (pIdx < 0) continue;
					p1 = particles	+ pIdx;

					if (p1->mPrevPos.GetW() >= 0.0f)
					{
						Vector4 localPos = TransformPoint(*reinterpret_cast<Vector4*>(&p1->mPos), v1->mInvTransform);

						// Unit sphere specific collision code
						float mag = localPos.GetX() * localPos.GetX() + localPos.GetY() * localPos.GetY() + localPos.GetZ() * localPos.GetZ();
						if (mag < baseRadiusSqr)
						{
							mag = (baseRadius / sqrtf(mag)) - 1.0f;

							repulse.Set(localPos.GetX() * mag, localPos.GetY() * mag, localPos.GetZ() * mag);
							repulse = TransformVector(repulse, v1->mTransform);

							float w = p1->mPos.GetW();
							p1->mPos += *reinterpret_cast<Vector4*>(&repulse);
							p1->mPos.SetW(w);

		/*						repulse *= 0.5f;

							w = p1->mPrevPos.GetW();
							p1->mPrevPos += *reinterpret_cast<Vector4*>(&repulse);
							p1->mPrevPos.SetW(w);
		*/					
						}
						// End Collision code
					}
				}
			}
		}
	}

	void ConstraintsSet::SphereSolve_vector (const Effect *effect) const
	{ 
		this->TubeSolve_scalar(effect);
	}

	void ConstraintsSet::CubeSolve_scalar (const Effect *effect) const {}
	void ConstraintsSet::CubeSolve_vector (const Effect *effect) const { this->CubeSolve_scalar(effect); }

	void ConstraintsSet::PlaneSolve_scalar (const Effect *effect) const {}
	void ConstraintsSet::PlaneSolve_vector (const Effect *effect) const { this->PlaneSolve_scalar(effect); }
}
