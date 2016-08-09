//  Copyright 2009 Electronic Arts, Inc. All rights reserved.
#ifndef _DATA_H_
#define _DATA_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <algorithm>

#define xASSERT assert

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
//typedef unsigned long long uint64_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
//typedef signed long long int64_t;

class EventID
{
public:
	EventID () : mCLPtr(NULL) {}
	EventID (void* clptr, const char* op) : mCLPtr(clptr) { this->PrintPre(op); this->PrintPost(); }	// note that this presumes object passed in has already been retained and needs release on destruction
	EventID (const EventID& src) : mCLPtr(src.mCLPtr) { this->Retain(); }
	~EventID () { this->Release(); }
	const EventID& operator= (const EventID& src) { this->Release(); mCLPtr = src.mCLPtr; this->Retain(); return *this; }
	void* Get () const { return mCLPtr; } 
	void** GetPtrTo () { return &mCLPtr; }
	bool IsValid () const { return mCLPtr != NULL; }
	void Wait ();
private:
	void Retain ();
	void Release ();
	void PrintPre (const char* msg);
	void PrintPost ();
	void* mCLPtr;
};

struct Vector4
{
	union {
		struct { float x,y,z,w; };
		float v[4];
	};
	
	Vector4 () : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {};
	Vector4 (float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {};
	void Set (float _x, float _y, float _z, float _w = 0.0f) { x = _x; y = _y; z = _z; w = _w; };
	void SetX (float _x) { x = _x; };
	void SetY (float _y) { y = _y; };
	void SetZ (float _z) { z = _z; };
	void SetW (float _w) { w = _w; };
	float GetX () const { return x; };
	float GetY () const { return y; };
	float GetZ () const { return z; };
	float GetW () const { return w; };
	const Vector4& operator += (const Vector4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; };
	const Vector4& operator -= (const Vector4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; };
	const Vector4& operator *= (const Vector4& rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this; };
	const Vector4& operator /= (const Vector4& rhs) { x /= rhs.x; y /= rhs.y; z /= rhs.z; w /= rhs.w; return *this; };
	const Vector4& operator += (float rhs) { x += rhs; y += rhs; z += rhs; w += rhs; return *this; };
	const Vector4& operator -= (float rhs) { x -= rhs; y -= rhs; z -= rhs; w -= rhs; return *this; };
	const Vector4& operator *= (float rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; };
	const Vector4& operator /= (float rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; };
};

struct Matrix44
{
	Vector4 m[4];
};

inline float Magnitude (const Vector4& v) { return sqrtf(v.x*v.x+v.y*v.y+v.z*v.z); };
inline float Dot (const Vector4& v1, const Vector4& v2) { return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z+v1.w*v2.w; };
inline float Dot3 (const Vector4& v1, const Vector4& v2) { return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z; };

inline Vector4 CrossProduct (const Vector4& v1, const Vector4& v2)
{
	return Vector4(
		v1.y * v2.z - v2.y * v1.z,
		v1.z * v2.x - v2.z * v1.x,
		v1.x * v2.y - v2.x * v1.y,
				   0.0f);
}

inline Vector4 operator+ (const Vector4& v1, const Vector4& v2)
{
	return Vector4(v1.x+v2.x,v1.y+v2.y,v1.z+v2.z,v1.w+v2.w);
}

inline Vector4 operator* (const Vector4& v, float scale)
{
	return Vector4(v.x * scale, v.y * scale, v.z * scale, v.w * scale);
}

inline Vector4 Normalize (const Vector4& v)
{
	float mag = Magnitude(v);
	float invmag = (mag > 0.000001f) ? (1.0f / mag) : 1.0f;
	return Vector4(v.x*invmag, v.y*invmag, v.z*invmag, 0.0f);
}

inline Vector4 TransformPoint (const Vector4& v, const Matrix44& m)
{
	return Vector4(
		v.x * m.m[0].x + v.y * m.m[1].x + v.z * m.m[2].x + m.m[3].x,
		v.x * m.m[0].y + v.y * m.m[1].y + v.z * m.m[2].y + m.m[3].y,
		v.x * m.m[0].z + v.y * m.m[1].z + v.z * m.m[2].z + m.m[3].z,
		v.x * m.m[0].w + v.y * m.m[1].w + v.z * m.m[2].w + m.m[3].w);
}

inline Vector4 TransformVector (const Vector4& v, const Matrix44& m)
{
	return Vector4(
		v.x * m.m[0].x + v.y * m.m[1].x + v.z * m.m[2].x,
		v.x * m.m[0].y + v.y * m.m[1].y + v.z * m.m[2].y,
		v.x * m.m[0].z + v.y * m.m[1].z + v.z * m.m[2].z,
		v.x * m.m[0].w + v.y * m.m[1].w + v.z * m.m[2].w);
}

inline void SetIdentity (Matrix44& m)
{
	m.m[0] = Vector4(1.0f,0.0f,0.0f,0.0f);
	m.m[1] = Vector4(0.0f,1.0f,0.0f,0.0f);
	m.m[2] = Vector4(0.0f,0.0f,1.0f,0.0f);
	m.m[3] = Vector4(0.0f,0.0f,0.0f,1.0f);
}

inline Matrix44 FastInverse (const Matrix44& a)
{
	Matrix44 result;
	result.m[0].x = a.m[0].x;	result.m[0].y = a.m[1].x;	result.m[0].z = a.m[2].x;	result.m[0].w = 0.0f;
	result.m[1].x = a.m[0].y;	result.m[1].y = a.m[1].y;	result.m[1].z = a.m[2].y;	result.m[1].w = 0.0f;
	result.m[2].x = a.m[0].z;	result.m[2].y = a.m[1].z;	result.m[2].z = a.m[2].z;	result.m[2].w = 0.0f;
	result.m[3].x = -a.m[3].x;	result.m[3].y = -a.m[3].y;	result.m[3].z = -a.m[3].z;	result.m[3].w = 0.0f;
	return result;
}

inline float Determinant (const Matrix44& m)
{
	float value = 
	m.m[0].v[3] * m.m[1].v[2] * m.m[2].v[1] * m.m[3].v[0]-m.m[0].v[2] * m.m[1].v[3] * m.m[2].v[1] * m.m[3].v[0]-m.m[0].v[3] * m.m[1].v[1] * m.m[2].v[2] * m.m[3].v[0]+m.m[0].v[1] * m.m[1].v[3] * m.m[2].v[2] * m.m[3].v[0]+
	m.m[0].v[2] * m.m[1].v[1] * m.m[2].v[3] * m.m[3].v[0]-m.m[0].v[1] * m.m[1].v[2] * m.m[2].v[3] * m.m[3].v[0]-m.m[0].v[3] * m.m[1].v[2] * m.m[2].v[0] * m.m[3].v[1]+m.m[0].v[2] * m.m[1].v[3] * m.m[2].v[0] * m.m[3].v[1]+
	m.m[0].v[3] * m.m[1].v[0] * m.m[2].v[2] * m.m[3].v[1]-m.m[0].v[0] * m.m[1].v[3] * m.m[2].v[2] * m.m[3].v[1]-m.m[0].v[2] * m.m[1].v[0] * m.m[2].v[3] * m.m[3].v[1]+m.m[0].v[0] * m.m[1].v[2] * m.m[2].v[3] * m.m[3].v[1]+
	m.m[0].v[3] * m.m[1].v[1] * m.m[2].v[0] * m.m[3].v[2]-m.m[0].v[1] * m.m[1].v[3] * m.m[2].v[0] * m.m[3].v[2]-m.m[0].v[3] * m.m[1].v[0] * m.m[2].v[1] * m.m[3].v[2]+m.m[0].v[0] * m.m[1].v[3] * m.m[2].v[1] * m.m[3].v[2]+
	m.m[0].v[1] * m.m[1].v[0] * m.m[2].v[3] * m.m[3].v[2]-m.m[0].v[0] * m.m[1].v[1] * m.m[2].v[3] * m.m[3].v[2]-m.m[0].v[2] * m.m[1].v[1] * m.m[2].v[0] * m.m[3].v[3]+m.m[0].v[1] * m.m[1].v[2] * m.m[2].v[0] * m.m[3].v[3]+
	m.m[0].v[2] * m.m[1].v[0] * m.m[2].v[1] * m.m[3].v[3]-m.m[0].v[0] * m.m[1].v[2] * m.m[2].v[1] * m.m[3].v[3]-m.m[0].v[1] * m.m[1].v[0] * m.m[2].v[2] * m.m[3].v[3]+m.m[0].v[0] * m.m[1].v[1] * m.m[2].v[2] * m.m[3].v[3];
	return value;
}

inline Matrix44 Inverse (const Matrix44& m) 
{
	Matrix44 result;
	result.m[0].v[0] = m.m[1].v[2]*m.m[2].v[3]*m.m[3].v[1] - m.m[1].v[3]*m.m[2].v[2]*m.m[3].v[1] + m.m[1].v[3]*m.m[2].v[1]*m.m[3].v[2] - m.m[1].v[1]*m.m[2].v[3]*m.m[3].v[2] - m.m[1].v[2]*m.m[2].v[1]*m.m[3].v[3] + m.m[1].v[1]*m.m[2].v[2]*m.m[3].v[3];
	result.m[0].v[1] = m.m[0].v[3]*m.m[2].v[2]*m.m[3].v[1] - m.m[0].v[2]*m.m[2].v[3]*m.m[3].v[1] - m.m[0].v[3]*m.m[2].v[1]*m.m[3].v[2] + m.m[0].v[1]*m.m[2].v[3]*m.m[3].v[2] + m.m[0].v[2]*m.m[2].v[1]*m.m[3].v[3] - m.m[0].v[1]*m.m[2].v[2]*m.m[3].v[3];
	result.m[0].v[2] = m.m[0].v[2]*m.m[1].v[3]*m.m[3].v[1] - m.m[0].v[3]*m.m[1].v[2]*m.m[3].v[1] + m.m[0].v[3]*m.m[1].v[1]*m.m[3].v[2] - m.m[0].v[1]*m.m[1].v[3]*m.m[3].v[2] - m.m[0].v[2]*m.m[1].v[1]*m.m[3].v[3] + m.m[0].v[1]*m.m[1].v[2]*m.m[3].v[3];
	result.m[0].v[3] = m.m[0].v[3]*m.m[1].v[2]*m.m[2].v[1] - m.m[0].v[2]*m.m[1].v[3]*m.m[2].v[1] - m.m[0].v[3]*m.m[1].v[1]*m.m[2].v[2] + m.m[0].v[1]*m.m[1].v[3]*m.m[2].v[2] + m.m[0].v[2]*m.m[1].v[1]*m.m[2].v[3] - m.m[0].v[1]*m.m[1].v[2]*m.m[2].v[3];
	result.m[1].v[0] = m.m[1].v[3]*m.m[2].v[2]*m.m[3].v[0] - m.m[1].v[2]*m.m[2].v[3]*m.m[3].v[0] - m.m[1].v[3]*m.m[2].v[0]*m.m[3].v[2] + m.m[1].v[0]*m.m[2].v[3]*m.m[3].v[2] + m.m[1].v[2]*m.m[2].v[0]*m.m[3].v[3] - m.m[1].v[0]*m.m[2].v[2]*m.m[3].v[3];
	result.m[1].v[1] = m.m[0].v[2]*m.m[2].v[3]*m.m[3].v[0] - m.m[0].v[3]*m.m[2].v[2]*m.m[3].v[0] + m.m[0].v[3]*m.m[2].v[0]*m.m[3].v[2] - m.m[0].v[0]*m.m[2].v[3]*m.m[3].v[2] - m.m[0].v[2]*m.m[2].v[0]*m.m[3].v[3] + m.m[0].v[0]*m.m[2].v[2]*m.m[3].v[3];
	result.m[1].v[2] = m.m[0].v[3]*m.m[1].v[2]*m.m[3].v[0] - m.m[0].v[2]*m.m[1].v[3]*m.m[3].v[0] - m.m[0].v[3]*m.m[1].v[0]*m.m[3].v[2] + m.m[0].v[0]*m.m[1].v[3]*m.m[3].v[2] + m.m[0].v[2]*m.m[1].v[0]*m.m[3].v[3] - m.m[0].v[0]*m.m[1].v[2]*m.m[3].v[3];
	result.m[1].v[3] = m.m[0].v[2]*m.m[1].v[3]*m.m[2].v[0] - m.m[0].v[3]*m.m[1].v[2]*m.m[2].v[0] + m.m[0].v[3]*m.m[1].v[0]*m.m[2].v[2] - m.m[0].v[0]*m.m[1].v[3]*m.m[2].v[2] - m.m[0].v[2]*m.m[1].v[0]*m.m[2].v[3] + m.m[0].v[0]*m.m[1].v[2]*m.m[2].v[3];
	result.m[2].v[0] = m.m[1].v[1]*m.m[2].v[3]*m.m[3].v[0] - m.m[1].v[3]*m.m[2].v[1]*m.m[3].v[0] + m.m[1].v[3]*m.m[2].v[0]*m.m[3].v[1] - m.m[1].v[0]*m.m[2].v[3]*m.m[3].v[1] - m.m[1].v[1]*m.m[2].v[0]*m.m[3].v[3] + m.m[1].v[0]*m.m[2].v[1]*m.m[3].v[3];
	result.m[2].v[1] = m.m[0].v[3]*m.m[2].v[1]*m.m[3].v[0] - m.m[0].v[1]*m.m[2].v[3]*m.m[3].v[0] - m.m[0].v[3]*m.m[2].v[0]*m.m[3].v[1] + m.m[0].v[0]*m.m[2].v[3]*m.m[3].v[1] + m.m[0].v[1]*m.m[2].v[0]*m.m[3].v[3] - m.m[0].v[0]*m.m[2].v[1]*m.m[3].v[3];
	result.m[2].v[2] = m.m[0].v[1]*m.m[1].v[3]*m.m[3].v[0] - m.m[0].v[3]*m.m[1].v[1]*m.m[3].v[0] + m.m[0].v[3]*m.m[1].v[0]*m.m[3].v[1] - m.m[0].v[0]*m.m[1].v[3]*m.m[3].v[1] - m.m[0].v[1]*m.m[1].v[0]*m.m[3].v[3] + m.m[0].v[0]*m.m[1].v[1]*m.m[3].v[3];
	result.m[2].v[3] = m.m[0].v[3]*m.m[1].v[1]*m.m[2].v[0] - m.m[0].v[1]*m.m[1].v[3]*m.m[2].v[0] - m.m[0].v[3]*m.m[1].v[0]*m.m[2].v[1] + m.m[0].v[0]*m.m[1].v[3]*m.m[2].v[1] + m.m[0].v[1]*m.m[1].v[0]*m.m[2].v[3] - m.m[0].v[0]*m.m[1].v[1]*m.m[2].v[3];
	result.m[3].v[0] = m.m[1].v[2]*m.m[2].v[1]*m.m[3].v[0] - m.m[1].v[1]*m.m[2].v[2]*m.m[3].v[0] - m.m[1].v[2]*m.m[2].v[0]*m.m[3].v[1] + m.m[1].v[0]*m.m[2].v[2]*m.m[3].v[1] + m.m[1].v[1]*m.m[2].v[0]*m.m[3].v[2] - m.m[1].v[0]*m.m[2].v[1]*m.m[3].v[2];
	result.m[3].v[1] = m.m[0].v[1]*m.m[2].v[2]*m.m[3].v[0] - m.m[0].v[2]*m.m[2].v[1]*m.m[3].v[0] + m.m[0].v[2]*m.m[2].v[0]*m.m[3].v[1] - m.m[0].v[0]*m.m[2].v[2]*m.m[3].v[1] - m.m[0].v[1]*m.m[2].v[0]*m.m[3].v[2] + m.m[0].v[0]*m.m[2].v[1]*m.m[3].v[2];
	result.m[3].v[2] = m.m[0].v[2]*m.m[1].v[1]*m.m[3].v[0] - m.m[0].v[1]*m.m[1].v[2]*m.m[3].v[0] - m.m[0].v[2]*m.m[1].v[0]*m.m[3].v[1] + m.m[0].v[0]*m.m[1].v[2]*m.m[3].v[1] + m.m[0].v[1]*m.m[1].v[0]*m.m[3].v[2] - m.m[0].v[0]*m.m[1].v[1]*m.m[3].v[2];
	result.m[3].v[3] = m.m[0].v[1]*m.m[1].v[2]*m.m[2].v[0] - m.m[0].v[2]*m.m[1].v[1]*m.m[2].v[0] + m.m[0].v[2]*m.m[1].v[0]*m.m[2].v[1] - m.m[0].v[0]*m.m[1].v[2]*m.m[2].v[1] - m.m[0].v[1]*m.m[1].v[0]*m.m[2].v[2] + m.m[0].v[0]*m.m[1].v[1]*m.m[2].v[2];

	float scale = (1.0f/Determinant(m));
	result.m[0] *= scale;
	result.m[1] *= scale;
	result.m[2] *= scale;
	result.m[3] *= scale;
	return result;
}

inline uint32_t tRGBA (float r, float g, float b, float a)
{
	uint32_t intR = (uint32_t)(r * 255.0) & 0xff;
	uint32_t intG = (uint32_t)(g * 255.0) & 0xff;
	uint32_t intB = (uint32_t)(b * 255.0) & 0xff;
	uint32_t intA = (uint32_t)(a * 255.0) & 0xff;
	return ((intR << 0) | (intG << 8) | (intB << 16) | (intA << 24));	// ABGR
}

namespace ROPA
{
	static const float Epsilon = 1e-5f;
	
	struct Particle
	{
		void Init(const Vector4& pos)	
		{
			// do not overwrite the W component
				mPos.Set(pos.GetX(),pos.GetY(),pos.GetZ(),mPos.GetW()); 
				mPrevPos.Set(pos.GetX(),pos.GetY(),pos.GetZ(),mPrevPos.GetW()); ;
		}

		void SetLocked(bool locked)		{mPrevPos.SetW((locked)?-1.0f:1.0f);}
		bool IsLocked() const			{return (mPrevPos.GetW()<0.0f)?true:false;}

		Vector4	mPos;			// x,y,z = position, w = invMass
		Vector4 mPrevPos;		// x,y,z = position, w>0 = friction && not locked particle, w< 0 = friction && locked particle
	};

	struct ParticleDebug
	{
		uint32_t	mColor;
		float		mLength;
	};

	
	struct VolumeTransform
	{
		Matrix44	mTransform;
		Matrix44	mInvTransform;
	};

	
	struct PosNormPair
	{
		Vector4	mPosition;		// w = vertex index in output buffer
		Vector4	mNormal;
	};

	
	struct VolumeUnitSpace
	{
		Matrix44	mUnitSpace;
	};

	
	struct ConstraintOctet
	{
		ConstraintOctet (void) { for (unsigned int i = 0; i < 8; ++i) { mParticleIdx[i] = mRefObjIdx[i] = -1; mMin[i] = mMax[i] = 0.0f; } };
		bool ContainsIdx (short idx) const { for (unsigned int i = 0; i < 8; ++i) { if ((mParticleIdx[i] == idx) || (mRefObjIdx[i] == idx)) return(true); } return(false); };
		int FindFirstUnused (void) const { unsigned int i; for (i = 0; (i < 8) && (mParticleIdx[i] >= 0); ++i) /* do nothing */; return(i); };

		ConstraintOctet& operator *=(float coef) {unsigned int i; for (i = 0; (i < 8) && (mParticleIdx[i] >= 0); ++i) {mMin[i] *= coef;mMax[i]*=coef;} return *this;}
		ConstraintOctet& operator +=(const ConstraintOctet& co) {unsigned int i; for (i = 0; (i < 8) && (mParticleIdx[i] >= 0); ++i) {mMin[i] += co.mMin[i];mMax[i] += co.mMax[i];} return *this;}
		bool SubAndMult(const ConstraintOctet& co, float coef) 
		{
			unsigned int i; 
			for (i = 0; (i < 8) && (mParticleIdx[i] >= 0); ++i) 
			{
				if ((mParticleIdx[i] != co.mParticleIdx[i]) || (mRefObjIdx[i] != co.mRefObjIdx[i]))
					return false;

				mMin[i] -= co.mMin[i];
				mMin[i] *= coef;
				mMax[i] -= co.mMax[i];
				mMax[i] *= coef;
			} 
			return true;
		}
		bool CheckParticlesIndices(int nbParticles) const
		{
			unsigned int i; 
			for (i = 0; (i < 8) && (mParticleIdx[i] >= 0); ++i) 
			{
				if (mParticleIdx[i] > nbParticles)
					return false;
			}
			return true;
		}
		short	mParticleIdx[8];
		short	mRefObjIdx[8];
		float	mMin[8];
		float	mMax[8];
	} ;

	
	struct ConstraintTuningParameters
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
	} ;

}

#endif // _DATA_H_
