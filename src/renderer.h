#ifndef RENDERER_H
#define RENDERER_H

#include <cstdint>
#include <memory>
#include <vector>

#include "math/vmath.h"
#include "math/common.h"

class Window;

struct Light
{
	Vector3d ambient;
	Vector3d diffuse;
	Vector3d specular;
	
	Vector3d pos;
	
	// Constant, linear and quadratic attenuation parameters.
	//double k0, k1, k2;
};

struct LightContext
{
	std::vector<Light> lights;
};

struct ShaderInput
{
	// 3d point in eye space
	Vector3d vert;
	
	// Normal in eye space
	Vector3d normal;
	
	// Texture coordinate
	Vector2i texCoord;
	
	// Screen coordinate (pixel on screen, including depth value)
	Vector3d screenCoord;
	
	// The light context
	std::shared_ptr<LightContext> lightContext;
};

class Renderer
{
public:
	using TWindowPtr = std::shared_ptr<Window>;
	using TShaderFunc = std::function<uint32_t(ShaderInput&)>;
	using TLightContextPtr = std::shared_ptr<LightContext>;
	
	Renderer(TWindowPtr window);
	~Renderer();
	
	void setFrustum(double fovY, double aspect, double near, double far);
	
	void setShader(TShaderFunc func) { mShader = func; }
	void setViewport(int32_t x, int32_t y, int32_t width, int32_t height)
	{
		mVX = x; mVY = y; mVWidth = width; mVHeight = height;
	}
	
	void setDepthRange(double near, double far)
	{
		mDepthNear = near; mDepthFar = far;
	}
	
	void setDepthCheck(bool depthCheck)
	{
		mDepthCheck = depthCheck;
	}
	
	void renderTriangle(const Triangle3d& triangle);
	
	void clearDepthBuffer();
	
private:
	using TDepthBuffer = std::vector<std::vector<double>>;
	
	TWindowPtr mWindow;
	TShaderFunc mShader;
	Matrix4f mProjection;
	
	// Viewport variables
	int32_t mVX, mVY, mVWidth, mVHeight;
	
	// Depth range (initally [0, 1])
	double mDepthNear, mDepthFar;
	bool mDepthCheck;
	TDepthBuffer mDepthBuffer;
	
	// Light context
	TLightContextPtr mLightContext;
	
	void projectToScreen(Triangle3d& screenTri, const Triangle3d& triangle);
	void clipToScreenSpace(Vector3d& screen, const Vector4d& pt);
	
	bool isInsideBoundries(const Vector3d& pt);
	bool isInsideBoundries(const Triangle3d& screenTri);
	
	void getRasterRegion(Box2i& region, const Triangle3d& screenTri);
	
	void raster(const Box2i& region, const Triangle3d& screenTri, const Triangle3d& triangle);
	void rasterPixel(ShaderInput& input);
};

#endif