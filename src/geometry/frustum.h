#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "plane.h"
#include "transform.h"
#include "viewport.h"
#include "../math/vmath.h"

// Does not support orthogonal projection
class Frustum
{
public:
	
	// Creates an eye-frustum
	Frustum(double fovY, double aspect, double near, double far);
	
	void setProjectionParams(double fovY, double aspect, double near, double far);
	
	// Can be modified!
	Transform& getTransform() { return mTransform; }
	const Transform& getTransform() const { return mTransform; }
	
	// Project point p in global space to normal device coordinates (NDC)
	Vector3d project(const Vector3d& p) const;
	// Project NDC to line in global space
	// Line unProject(double x, double y) const;
	
	Vector3d ndcToViewportSpace(const Vector3d& ndc, const Viewport& viewport) const;
	
	// Normal device coordinates ([-1, 1]) contained within frustum
	static bool ndcContained(const Vector3d& ndc, double epsilon = 1e-3);
	
	// Primitives contained completely within this frustum
	bool contains(const Vector3d& p) const;
	
	// Get width of rectangles
	double getNearWidth() const;
	double getNearHeight() const;
	double getHalfNearWidth() const;
	double getHalfNearHeight() const;
	
	double getFarWidth() const;
	double getFarHeight() const;
	double getHalfFarWidth() const;
	double getHalfFarHeight() const;
	
	// Expressed in global coordinates
	Plane getNearPlane() const;
	Plane getFarPlane() const;
	Plane getLeftPlane() const;
	Plane getRightPlane() const;
	Plane getTopPlane() const;
	Plane getBottomPlane() const;
	
	// These methods have the input rages [-1, 1]
	Vector3d getNearPos(double x, double y) const;
	Vector3d getFarPos(double x, double y) const;
	
	Vector3d getCorner(int idx) const;
	int getCornerCount() const { return 8; }
	
	
private:
	void projectionChanged();
	
	double mFovY, mAspect, mNear, mFar;
	Transform mTransform;
	Matrix4d mProjection;
	
};

#endif