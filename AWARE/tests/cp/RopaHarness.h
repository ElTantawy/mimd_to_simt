/*
 *  RopaHarness.h
 *  Ropa
 *
 *  Created by Andrew on 09-02-12.
 *  Copyright 2009 Electronic Arts, Inc. All rights reserved.
 *
 */

#include "Data.h"
#include <GL/gl.h>
#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#include <CL/cl.h>

namespace ROPA
{ 
	class Effect; 
};

struct RopaBinaryHeader;
class RopaConstructionData;
class RopaOpenCL;

typedef struct {
	GLdouble x,y,z;
} recVec;

typedef struct {
	recVec viewPos; // View position
	recVec viewDir; // View direction vector
	recVec viewUp; // View up direction
	recVec rotPoint; // Point to rotate about
	GLdouble aperture; // pContextInfo->camera aperture
	GLint viewWidth, viewHeight; // current window/screen height and width
} recCamera;

typedef struct DistanceSolverOptions_struct {
   DistanceSolverOptions_struct() 
   : useAtomic(false), useTM(false), useTMOpt(false)  
   {}

   bool useAtomic; // use Fine-Grained Lock kernel 
   bool useTM; // use the TM kernel 
   bool useTMOpt; // use TM kernel with optimizations 
} DistanceSolverOptions; 

class RopaHarness 
{
public:
	RopaHarness (void *glContext, const char* opencl_code, DistanceSolverOptions distanceSolverOpts, int local_ws, bool enableOutputDump);
	~RopaHarness ();
	
	void Load (const char* filename);
	
	void SetMode(bool opencl, int kernelmode, bool usegpu, bool profile, bool usemixedmode);
	
	void UserImpulse (float x, float y);
	void ResetCurrentFrame ();
	void ComputeFrame ( int cycle );
	
	void ResetCamera ();
	void RotateObject (float angle, float x, float y, float z);
	void SetViewport (float width, float height);
	void Render () const;

	//---------------------------------

	void SetCamera () const;
	void SetLighting () const;
	GLuint NumTris () const { return mNumTris; };
	
	//---------------------------------

	void AddReplication (float x, float y, float z);
	
	void PrepareToConstruct ();
	void InitEffect ();
	void* GetSaveData () const;
	size_t GetSaveDataSize () const;
	
	void AddParticlePos (const Vector4& pos);
	void AddParticlePrevPos (const Vector4& pos);
	
	void AddDriverPosition (const Vector4& pos);
	void AddDriverNormal (const Vector4& pos);
	
	void AddVolumeTransform (const Matrix44& m);
	void AddVolumeInverse (const Matrix44& m);
	
	void AddVertexMapElement (int v);
	void AddMappedVertexElement (int v);
	void AddReverseMappingElement (int v);
	void AddVertexIndex(int v);

	void AddConstraintsSet ();
	void AddConstraintOctet (const ROPA::ConstraintOctet& co);
	void SetConstraintsSetIterations (int iters);
	void SetConstraintsSetSolver (int solver);
	void SetConstraintsSetObject (int index);
	
	void SetDT (float dt);
	void SetLastDT (float dt);
	void SetLastFullDT (float dt);
	void SetDistanceCDIndex (int i);
	void SetDriverCDIndex (int i);
	void SetSystemPosition (const Matrix44& m);
	void SetDeltaPos (const Matrix44& m);
	void SetVertexPositionPackingOffset (const Vector4& m);
	void SetGravity (const Vector4& m);
	
	void AddFramePosition (const Matrix44& m, float dt);

private:
	void DestructLoadData ();
	void SetupCLEffect ();
	void ComputePoseAndCentroid ();
	void ComputeNormalsFromPose ();
	
private:
	RopaConstructionData	*mConstructData;
	RopaOpenCL				*mCLInterface;
	
	char					*mBinaryData;
	size_t					mBinaryDataSize;
	
	RopaBinaryHeader		*mHeader;
	bool					mNeedDelete;
	
	ROPA::Effect			*mRopaEffect;
	Vector4					*mVertexTransferBuffer;
	Vector4					*mVertexSourcePose;
	Vector4					mCentroid;
	Matrix44*				mFrames;
	const float*			mFrameTimes;
	int						mNumFrames;
	int						mCurrentFrame;
	recCamera				mCamera;
	float					mShapeSize;
	float					mObjectRotation[4];
	float					mUserImpulse[2];
	GLuint					mNumTris;
	GLuint					mNumAllocVerts;
	GLuint					mIndices;
	GLuint					mNormals;
	GLuint					mVertices[2];		// 3 floats + 4 byte colour
	int						mCurrentComputeBuffer;
	int						mCurrentDrawVerts;
	int						mKernelMode;
	bool					mMappedVerts;
	bool					mWaitAfterCompute;
	bool					mHaveBuffers;
	bool					mHaveUserImpulse;
	bool					mUseOpenCL;
	bool					mUseGPU;

	bool                    mEnableOutputDump;
};
