#include "frustum.h"

Frustum::Frustum(double fovY, double aspect, double near, double far) :
	mFovY(fovY),
	mAspect(aspect),
	mNear(near),
	mFar(far)
{
	projectionChanged();
}
void Frustum::setProjectionParams(double fovY, double aspect, double near, double far)
{
	mFovY = fovY;
	mAspect = aspect;
	mNear = near;
	mFar = far;
	
	projectionChanged();
}

void Frustum::projectionChanged()
{
	double h = getHalfNearHeight();
	double w = getHalfNearWidth();
	
	mProjection = Matrix4d::createFrustum(-w, w, -h, h, mNear, mFar);
}

double Frustum::getNearWidth() const
{
	return getNearHeight() * mAspect;
}
double Frustum::getNearHeight() const
{
	return tan( mFovY / 360.0 * M_PI ) * mNear * 2.0;
}

double Frustum::getHalfNearWidth() const
{
	return getHalfNearHeight() * mAspect;
}
double Frustum::getHalfNearHeight() const
{
	return tan( mFovY / 360.0 * M_PI ) * mNear;
}

double Frustum::getFarWidth() const
{
	return getFarHeight() * mAspect;
}
double Frustum::getFarHeight() const
{
	return tan( mFovY / 360.0 * M_PI ) * mFar * 2.0;
}
double Frustum::getHalfFarWidth() const
{
	return getHalfFarHeight() * mAspect;
}
double Frustum::getHalfFarHeight() const
{
	return tan( mFovY / 360.0 * M_PI ) * mFar;
}

Vector3d Frustum::project(const Vector3d& p) const
{
	// From world to local coordinate system
	Vector3d local = mTransform.globalToLocal(p);
	
	// Get homogeneous projection onto near plane
	Vector4d proj = mProjection * Vector4d(local, 1.0);
	
	// Return normal device coordinates 
	return proj.xyz() / proj.w;
}

Vector3d Frustum::ndcToViewportSpace(const Vector3d& ndc, const Viewport& viewport) const
{
	double halfWidth = viewport.getWidth() / 2.0;
	double halfHeight = viewport.getHeight() / 2.0;
	double fn = (viewport.getDepthFar() - viewport.getDepthNear()) / 2.0;
	double nf = (viewport.getDepthNear() - viewport.getDepthFar()) / 2.0;
	
	return Vector3d(ndc.x * halfWidth + (viewport.getX() + halfWidth), 
					ndc.y * halfHeight + (viewport.getY() + halfHeight),
					ndc.z * fn + nf);
}

bool Frustum::ndcContained(const Vector3d& ndc, double epsilon)
{
	double pos = 1.0 + epsilon;
	double neg = -pos;
	return neg <= ndc.x && ndc.x <= pos &&
		   neg <= ndc.y && ndc.y <= pos &&
		   neg <= ndc.z && ndc.z <= pos;
}

bool Frustum::contains(const Vector3d& p) const
{
	return ndcContained(project(p));
}

// Project NDC to line in global space
// Line unProject(double x, double y) const;

// Expressed in global coordinates
Plane Frustum::getNearPlane() const
{
	const Vector3d& pos = mTransform.getPosition();
	const Vector3d& at = mTransform.getAt();
	return Plane(pos + at * mNear, -at);
}
Plane Frustum::getFarPlane() const
{
	const Vector3d& pos = mTransform.getPosition();
	const Vector3d& at = mTransform.getAt();
	return Plane(pos + at * mNear, at);
}
Plane Frustum::getLeftPlane() const
{
	Vector3d left = -mTransform.getRight() * getHalfNearWidth();
	Vector3d leftSide = mTransform.getAt() + left;
	Vector3d normal = mTransform.getUp().crossProduct(leftSide);
	return Plane(mTransform.getPosition(), normal);
}
Plane Frustum::getRightPlane() const
{
	Vector3d right = mTransform.getRight() * getHalfNearWidth();
	Vector3d rightSide = mTransform.getAt() + right;
	Vector3d normal = rightSide.crossProduct(mTransform.getUp());
	return Plane(mTransform.getPosition(), normal);
}
Plane Frustum::getTopPlane() const
{
	Vector3d top = mTransform.getUp() * getHalfNearHeight();
	Vector3d topSide = mTransform.getAt() + top;
	Vector3d normal = topSide.crossProduct(-mTransform.getRight());
	return Plane(mTransform.getPosition(), normal);
}
Plane Frustum::getBottomPlane() const
{
	Vector3d bottom = -mTransform.getUp() * getHalfNearHeight();
	Vector3d bottomSide = mTransform.getAt() + bottom;
	Vector3d normal = bottomSide.crossProduct(mTransform.getRight());
	return Plane(mTransform.getPosition(), normal);
}

// These methods have the input rages [-1, 1]
Vector3d Frustum::getNearPos(double x, double y) const
{
	const Vector3d& pos = mTransform.getPosition();
	const Vector3d& at = mTransform.getAt();
	const Vector3d& right = mTransform.getRight();
	const Vector3d& up = mTransform.getUp();
	
	const Vector3d center = pos + at * mNear;
	const Vector3d weight = right * x * getHalfNearWidth() + up * y * getHalfNearHeight();
	return center + weight;
}
Vector3d Frustum::getFarPos(double x, double y) const
{
	const Vector3d& pos = mTransform.getPosition();
	const Vector3d& at = mTransform.getAt();
	const Vector3d& right = mTransform.getRight();
	const Vector3d& up = mTransform.getUp();
	
	const Vector3d center = pos + at * mFar;
	const Vector3d weight = right * x * getHalfFarWidth() + up * y * getHalfFarHeight();
	return center + weight;
}

Vector3d Frustum::getCorner(int idx) const
{
	switch(idx)
	{
	default:
	case 0: return getNearPos(-1.0, -1.0);
	case 1: return getNearPos(-1.0,  1.0);
	case 2: return getNearPos( 1.0,  1.0);
	case 3: return getNearPos( 1.0, -1.0);
	case 4: return getFarPos ( 1.0, -1.0);
	case 5: return getFarPos (-1.0, -1.0);
	case 6: return getFarPos (-1.0,  1.0);
	case 7: return getFarPos ( 1.0,  1.0);
	}
}