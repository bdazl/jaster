#ifndef PLANE_H
#define PLANE_H

#include <cstdint>
#include "../math/vmath.h"

class Plane
{
public:
	using TVec3 = Vector3d;
	
	Plane(const TVec3& pt, const TVec3& normal) :
		mNormal(normal),
		mPoint(pt)
	{
		construct();
	}
	
	// Construct plane from three points.
	// Order matters; right hand rule can be
	// used to determine the direction of the normal
	Plane(const TVec3& p0, const TVec3& p1, const TVec3& p2) :
		mNormal((p1 - p0).crossProduct(p2 - p1)),
		mPoint(p0)
	{
		construct();
	}
	
	double signedDistance(const TVec3& point)
	{
		// Since the plane is always kept in hessian normal form.
		return mNormal.dotProduct(point) + mD;
	}
	
private:
	void construct()
	{
		mNormal.normalize();
		mD = -(mNormal.dotProduct(mPoint));
	}
	
	// normal.x * x + normal.y * y + normal.z * z = -d
	TVec3 mNormal;
	TVec3 mPoint;
	double mD;
};

#endif