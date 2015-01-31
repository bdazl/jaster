#ifndef PLANE_H
#define PLANE_H

#include <cstdint>
#include "vmath.h"

template<typename T>
class Plane3
{
public:
	using TVec3 = Vector3<T>;
	
	// Construct plane from three points.
	// Order matters; right hand rule can be
	// used to determine the direction of the normal
	Plane3(const TVec3& p0, const TVec3& p1, const TVec3& p2)
	{
		mNormal = (p1 - p0).crossProduct(p2 - p1);
		mNormal.normalize();
		mPoint = p0;
		mD = -(mNormal.dotProduct(mPoint));
	}
	
	T signedDistance(const TVec3& point)
	{
		// Since the plane is always kept in hessian normal form.
		return mNormal.dotProduct(point - mPoint);
	}
	
private:
	// normal.x * x + normal.y * y + normal.z * z = -d
	TVec3 mNormal;
	TVec3 mPoint;
	T mD;
};

using Plane3d = Plane3<double>;

#endif