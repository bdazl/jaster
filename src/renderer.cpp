#include "renderer.h"
#include "Window.h"
#include <algorithm>

namespace 
{
	const double xcNear = 0.12;
	const double xcFar = 100.0;
	
	uint32_t standardShader(ShaderInput& input)
	{
		return 0xFF0000;
	}
}

Renderer::Renderer(TWindowPtr window) :
	mWindow(window),
	mShader(standardShader),
	mProjection(),
	mVX(0),
	mVY(0),
	mVWidth(mWindow->getWidth()),
	mVHeight(mWindow->getHeight()),
	mDepthNear(0.0),
	mDepthFar(1.0)
{
	double halfWidth = mWindow->getWidth() / 2.0;
	double halfHeight = mWindow->getHeight() / 2.0;
	mProjection = Matrix4d::createFrustum(-halfWidth,
										   halfWidth,
										   halfHeight,
										  -halfHeight, xcNear, xcFar);
}

Renderer::~Renderer()
{
}

void Renderer::renderTriangle(const Triangle3d& triangle)
{
	// TODO: Clip to frustum
	
	Triangle3d screenTri;
	projectToScreen(screenTri, triangle);
	
	if (!isInsideBoundries(screenTri))
	{
		return;
	}
	
	// TODO: Check if facing towards us
	
	Box2i region;
	getRasterRegion(region, screenTri);
	raster(region, screenTri, triangle);
}

void Renderer::projectToScreen(Triangle3d& screenTri, const Triangle3d& triangle)
{
	Vector4d clip0 = mProjection * Vector4d(triangle.p0, 1.0);
	Vector4d clip1 = mProjection * Vector4d(triangle.p1, 1.0);
	Vector4d clip2 = mProjection * Vector4d(triangle.p2, 1.0);
	
	// The coords are still homogeneous and needs to be mapped to screen space.
	clipToScreenSpace(screenTri.p0, clip0);
	clipToScreenSpace(screenTri.p1, clip1);
	clipToScreenSpace(screenTri.p2, clip2);
}

void Renderer::clipToScreenSpace(Vector3d& screen, const Vector4d& pt)
{
	double halfWidth = mWindow->getWidth() / 2.0;
	double halfHeight = mWindow->getHeight() / 2.0;
	double fn = (mDepthFar - mDepthNear) / 2.0;
	double nf = (mDepthNear - mDepthFar) / 2.0;
	
	// Normalized device coordinates
	Vector3d ndc = pt.xyz() / pt.w;
	
	// Window (or screen) coordinates
	screen = Vector3d(ndc.x * halfWidth + (mVX + halfWidth), 
					  ndc.y * halfHeight + (mVY + halfHeight),
					  ndc.z * fn + nf);
}

bool Renderer::isInsideBoundries(const Vector3d& pt)
{
	return pt.x > 0 && pt.x < mWindow->getWidth() &&
		   pt.y > 0 && pt.y < mWindow->getHeight();
}

bool Renderer::isInsideBoundries(const Triangle3d& screenTri)
{
	return isInsideBoundries(screenTri.p0) ||
		   isInsideBoundries(screenTri.p1) ||
		   isInsideBoundries(screenTri.p2);
}

void Renderer::getRasterRegion(Box2i& region, const Triangle3d& screenTri)
{
	region.p0.x = (int)std::min(screenTri.p0.x, std::min(screenTri.p1.x, screenTri.p2.x));
	region.p0.y = (int)std::min(screenTri.p0.y, std::min(screenTri.p1.y, screenTri.p2.y));
	
	region.p1.x = (int)std::ceil(std::max(screenTri.p0.x, std::max(screenTri.p1.x, screenTri.p2.x)));
	region.p1.y = (int)std::ceil(std::max(screenTri.p0.y, std::max(screenTri.p1.y, screenTri.p2.y)));
}

void Renderer::raster(const Box2i& region, const Triangle3d& screenTri, const Triangle3d& triangle)
{
	const int minY = std::max(region.p0.y, 0);
	const int endY = std::min(region.p1.y + 1, mWindow->getHeight());
	const int minX = std::max(region.p0.x, 0);
	const int endX = std::min(region.p1.x + 1, mWindow->getHeight());
	
	ShaderInput shaderInput;
	
	for (int y = minY; y < endY; y++)
	{
		for (int x = minX; x < endX; x++)
		{
			// TODO: Interpolate depth for this pixel.
			double depth = screenTri.p0.z;
			
			Vector2d coord((double)x, (double)y);
			if (math::pointIsInsideScreenTriangle(screenTri, coord))
			{
				// TODO: Depth test
				// TODO: Project 3d-coord
				// TODO: Texture coord
				shaderInput.screenCoord = Vector3d(coord.x, coord.y, depth);
				rasterPixel(shaderInput);
			}
		}
	}
}

void Renderer::rasterPixel(ShaderInput& shaderInput)
{
	uint32_t color = mShader(shaderInput);
	// TODO: Blend func
	
	mWindow->putPixel(shaderInput.screenCoord.x, shaderInput.screenCoord.y, color);
}