#ifndef TRANSFORM_H
#define TRANSFORM_H

class Transform
{
public:
	Transform() :
		mPos(0.0, 0.0, 0.0),
		mRot(1.0, 0.0, 0.0, 0.0)
	{
		
	}
	Transform(const Vector3d& pos, const Quatd& rot) :
		mPos(pos),
		mRot(rot)
	{	
	}
	
	// Always rotate relative to current rotation, in local space.
	void rotate(const Quatd& rot)
	{
		mRot = rot * mRot;
		mRot.normalize();
	}
	void setRotation(const Quatd& rot)
	{
		mRot = rot;
	}
	const Quatd& getRotation() const
	{
		return mRot;
	}
	
	void translate(const Vector3d& translate)
	{
		mPos += translate;
	}
	
	void setPosition(const Vector3d& pos)
	{
		mPos = pos;
	}
	
	const Vector3d& getPosition() const
	{
		return mPos;
	}
	
	Vector3d getRight() const
	{
		return mRot.rotate(Vector3d(1.0, 0.0, 0.0));
	}
	
	Vector3d getUp() const
	{
		return mRot.rotate(Vector3d(0.0, 1.0, 0.0));
	}
	
	Vector3d getAt() const
	{
		return mRot.rotate(Vector3d(0.0, 0.0, 1.0));
	}
	
	Transform getInverse() const
	{
		return Transform(-mPos, ~mRot);
	}
	
	// Go from a point expressed in local coordinates 
	// to a point in the global reference frame
	Vector3d localToGlobal(const Vector3d& v) const
	{
		return mRot.rotate(v) + mPos;
	}
	
	Vector3d globalToLocal(const Vector3d& v) const
	{
		return mRot.inverseRotate(v - mPos);
	}
	
	// The same as: T * R * rhs.T * rhs.R
	Transform& operator*(const Transform& rhs)
	{
		mPos += mRot.rotate(rhs.mPos);
		mRot = mRot * rhs.mRot;
		
		mRot.normalize();
		return *this;
	}
	
private:
	Vector3d mPos;
	Quatd mRot;
};

#endif