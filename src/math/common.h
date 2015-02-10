#ifndef COMMON_H
#define COMMON_H

#include "vmath.h"

struct Triangle3d
{
	// Vert
	Vector3d p0, p1, p2;

	// Normal
	Vector3d n0, n1, n2;
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
	Vector3d transform(const Matrix4d& transform, const Vector4d& pt);
	void transform(Triangle3d& out, const Matrix4d& transform, const Triangle3d& in);

	void clamp(double& out, double min, double max);
	// Clamp all components of the vector to min and max values.
	void clamp(Vector3d& vec, double min, double max);
	
	// Normal must be normalized
	Vector3d reflect(const Vector3d& d, const Vector3d& normal);
}

#endif