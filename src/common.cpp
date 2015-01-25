#include "common.h"

double math::sign(const Vector2d& p0, const Vector2d& p1, const Vector2d& p2)
{
	return (p0.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
}

bool math::pointIsInsideScreenTriangle(const Triangle3d& tri, const Vector2d& pt)
{
	bool b1, b2, b3;

	b1 = sign(pt, tri.p0.xy(), tri.p1.xy()) < 0.0f;
	b2 = sign(pt, tri.p1.xy(), tri.p2.xy()) < 0.0f;
	b3 = sign(pt, tri.p2.xy(), tri.p0.xy()) < 0.0f;

	return ((b1 == b2) && (b2 == b3));
}

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