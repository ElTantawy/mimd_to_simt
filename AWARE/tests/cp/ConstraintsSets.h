//  Copyright 2009 Electronic Arts, Inc. All rights reserved.
#ifndef _CONSTRAINTSSETS_H_
#define _CONSTRAINTSSETS_H_

#include "Data.h"

namespace ROPA
{
	enum ESolverIdent
	{
		kNullSolver,
		kParticleDistanceSolver,
		kDriverSolver,
		kSphereSolver,
		kTubeSolver,
		kCubeSolver,
		kPlaneSolver,

		kNumSolvers
	};

	enum ESolverMode
	{
		kScalarSolverMode,
		kVectorSolverMode,
		kTestSolverMode
	};

	class Effect;
	struct ConstraintSolver { virtual EventID operator() (EventID event, int constraintIndex, int numOctets, int numIters, short refObjIdx)=0; };
	
	class ConstraintsSet
	{
	public:
		ConstraintsSet() : mArray(0), mNbConstraintOctets(0), mNbIterations(1), mSolverIdent(0), mRefObjIndex(0) {}
		~ConstraintsSet() {}

		void SetSolverIdent (ESolverIdent si)			{ mSolverIdent = (short)si; }
		void SetConstraintsArray(ConstraintOctet* constraints, int nbConstraintOctets) { mArray = constraints; this->SetNbConstraints(nbConstraintOctets); }
		void SetNbConstraints(int nbConstraintOctets)	{ xASSERT((nbConstraintOctets < 65536) && nbConstraintOctets >= 0); mNbConstraintOctets = nbConstraintOctets; }
		void SetNbIterations(int nbIter)				{ xASSERT((nbIter < 256) && nbIter > 0); mNbIterations = nbIter; }
		void SetRefObjIndex(int refObjIdx)				{ mRefObjIndex = refObjIdx; }

		ConstraintOctet* GetConstraintsArray()	const							{ return mArray; }
		int				 GetNbConstraintOctets() const							{ return mNbConstraintOctets; }
		int				 GetParticleIndex(int octetIndex, int subIndex) const	{ return mArray[octetIndex].mParticleIdx[subIndex]; }
		ESolverIdent	 GetSolverIdent() const									{ return (ESolverIdent)mSolverIdent; }

		void UpdateParticleDistanceConstraints(const Effect* effect, const Vector4& scale,void* positions, size_t stride);
		EventID Solve (EventID event, const Effect* effect, int index, ESolverMode mode, ConstraintSolver* cs) const;
		void FinishInitialization (int nbParticles, int nbVolumes);

	protected:
		EventID TestSolver (EventID event, const Effect* effect, int index, ESolverMode mode, ConstraintSolver* cs) const;

		// different kinds of solvers
		void ParticleDistanceSolve_scalar (const Effect* effect) const;
		void ParticleDistanceSolve_vector (const Effect* effect) const;

		void DriverSolve_scalar (const Effect* effect) const;
		void DriverSolve_vector (const Effect* effect) const;

		void SphereSolve_scalar (const Effect* effect) const;
		void SphereSolve_vector (const Effect* effect) const;

		void TubeSolve_scalar (const Effect* effect) const;
		void TubeSolve_vector (const Effect* effect) const;

		void CubeSolve_scalar (const Effect* effect) const;
		void CubeSolve_vector (const Effect* effect) const;

		void PlaneSolve_scalar (const Effect* effect) const;
		void PlaneSolve_vector (const Effect* effect) const;

		// data members
		ConstraintOctet*	mArray;
		int					mNbConstraintOctets;
		int					mNbIterations;
		short				mSolverIdent;
		short				mRefObjIndex;
	};
}

#endif // _CONSTRAINTSSETS_H_
