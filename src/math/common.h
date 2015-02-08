#ifndef COMMON_H
#define COMMON_H

#include "vmath.h"

struct Triangle3d
{
	Vector3d p0, p1, p2;
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
	Vector3d transform(const Matrix4d& transform, const Vector3d& pt);
	void transform(Triangle3d& out, const Matrix4d& transform, const Triangle3d& in);
}

#endif