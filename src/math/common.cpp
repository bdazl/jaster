#include "common.h"

Vector3d math::transform(const Matrix4d& transform, const Vector4d& pt)
{
	return (transform * pt).xyz();
}

static inline Vector4d point(const Vector3d& pt)
{
	return Vector4d(pt.x, pt.y, pt.z, 1.0);
}

static inline Vector4d vect(const Vector3d& pt)
{
	return Vector4d(pt.x, pt.y, pt.z, 0.0);
}

void math::transform(Triangle3d& out, const Matrix4d& transform, const Triangle3d& tri)
{
	out.p0 = math::transform(transform, point(tri.p0));
	out.n0 = math::transform(transform, vect(tri.n0));
	
	out.p1 = math::transform(transform, point(tri.p1));
	out.n1 = math::transform(transform, vect(tri.n1));
	
	out.p2 = math::transform(transform, point(tri.p2));
	out.n2 = math::transform(transform, vect(tri.n2));
}

void math::clamp(double& out, double min, double max)
{
	if (out < min)
	{
		out = min;
	}
	if (out > max)
	{
		out = max;
	}
}

void math::clamp(Vector3d& vec, double min, double max)
{
	clamp(vec.x, min, max);
	clamp(vec.y, min, max);
	clamp(vec.z, min, max);
}

Vector3d math::reflect(const Vector3d& d, const Vector3d& normal)
{
	return d - normal * 2*(d.dotProduct(normal));
}