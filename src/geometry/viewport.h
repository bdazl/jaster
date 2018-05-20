#ifndef VIEWPORT_H
#define VIEWPORT_H

class Viewport
{
public:
	Viewport(double width, double height) :
		mWidth(width),
		mHeight(height),
		mDepthNear(0.0),
		mDepthFar(1.0),
		mX(0.0),
		mY(0.0)
	{
	}
	
	double getWidth() const { return mWidth; }
	double getHeight() const { return mHeight; }
	double getDepthNear() const { return mDepthNear; }
	double getDepthFar() const { return mDepthFar; }
	double getX() const { return mX; }
	double getY() const { return mY; }
private:
	double mWidth, mHeight, mDepthNear, mDepthFar, mX, mY;
};

#endif