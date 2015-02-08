#include "common.h"

Vector3d math::transform(const Matrix4d& transform, const Vector3d& pt)
{
	return (transform * Vector4d(pt.x, pt.y, pt.z, 1.0)).xyz();
}

void math::transform(Triangle3d& out, const Matrix4d& transform, const Triangle3d& tri)
{
	out.p0 = math::transform(transform, tri.p0);
	out.p1 = math::transform(transform, tri.p1);
	out.p2 = math::transform(transform, tri.p2);
}
