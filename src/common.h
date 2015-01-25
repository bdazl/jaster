#ifndef COMMON_H
#define COMMON_H

#include "vmath.h"

struct Triangle3d
{
	Vector3d p0, p1, p2;
};

struct Triangle2d
{
	Vector2d p0, p1, p2;
};

template<typename T>
struct Box2
{
	Vector2<T> p0, p1;
};

typedef Box2<double> Box2d;
typedef Box2<int> Box2i;

namespace math
{
	double sign(const Vector2d& p0, const Vector2d& p1, const Vector2d& p2);
	bool pointIsInsideScreenTriangle(const Triangle3d& tri, const Vector2d& pt);
	
	void transform(Triangle3d& out, const Triangle3d& in, const Matrix4d& transform);
}

#endif