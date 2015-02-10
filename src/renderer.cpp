#include "renderer.h"
#include "Window.h"
#include "math/plane.h"
#include <algorithm>

namespace 
{
	const double xcNear = 1.0;
	const double xcFar = 1000.0;
	
	uint32_t colorVecToUint(const Vector3d& color)
	{
		return uint32_t(color.r * 0xFF) << 16 | 
			   uint32_t(color.g * 0xFF) << 8 | 
			   uint32_t(color.b * 0xFF); 
	}
	
	uint32_t standardShader(ShaderInput& input)
	{
		// double dist = 1.0 + input.screenCoord.z;
		// TODO:: material property
		const double shiny = 0.2;
		
		Vector3d color(0.0, 0.0, 0.0);
		for (const auto& light : input.lightContext->lights)
		{
			Vector3d dir = input.vert - light.pos;
			dir.normalize();
			
			Vector3d toEye = -input.vert;
			toEye.normalize();
			
			Vector3d reflect = math::reflect(dir, input.normal);
			reflect.normalize();
			
			// Diffuse term
			Vector3d diff = light.diffuse * std::max(dir.dotProduct(input.normal), 0.0);
			math::clamp(diff, 0.0, 1.0);
			
			// Specular term
			double f = std::max(reflect.dotProduct(toEye), 0.0);
			Vector3d spec = light.specular * std::pow(f, shiny);
			math::clamp(spec, 0.0, 1.0);
			
			color += light.ambient + diff + spec;
		}
		
		math::clamp(color, 0.0, 1.0);
		
		return colorVecToUint(color);
	}
}

Renderer::Renderer(TWindowPtr window) :
	mWindow(window),
	mShader(standardShader),
	mProjection(),
	mVX(0),
	mVY(0),
	mVWidth(window->getWidth()),
	mVHeight(window->getHeight()),
	mDepthNear(0.0),
	mDepthFar(1.0),
	mDepthCheck(true),
	mDepthBuffer()
{
	setFrustum(2.0 * atan(mWindow->getHeight() / 2.0 / xcNear), mWindow->getWidth() / (double)mWindow->getHeight(), xcNear, xcFar);
	
	for (int y = 0; y < window->getHeight(); y++)
	{
		mDepthBuffer.push_back(std::vector<double>());
		for (int x = 0; x < window->getWidth(); x++)
		{
			mDepthBuffer[y].push_back(-mDepthFar);
		}
	}
	
	// TODO:: LET CLIENTS HANDLE THIS
	mLightContext = std::make_shared<LightContext>();
	
	Light l;
	l.pos = Vector3d(-100.0, 100.0, -50.0);
	// l.dir = Vector3d(0.0, -1.0, 0.0);
	
	l.ambient = Vector3d(0.05, 0.05, 0.05);
	l.diffuse = Vector3d(0.4, 0.4, 0.4);
	l.specular = Vector3d(0.4, 0.2, 0.2);
	
	mLightContext->lights.push_back(l);
}

Renderer::~Renderer()
{
}

void Renderer::setFrustum(double fovY, double aspect, double near, double far)
{
	double w, h;
	
	h = tan( fovY / 360.0 * M_PI ) * near;
	w = h * aspect;
	
	mProjection = Matrix4d::createFrustum(-w, w, -h, h, near, far);
}

void Renderer::clearDepthBuffer()
{
	double clr = -mDepthFar;
	for (auto& it : mDepthBuffer)
	{
		std::for_each(it.begin(), it.end(), [clr](double& d){ d = clr; });
	}
}

void Renderer::renderTriangle(const Triangle3d& triangle)
{
	Plane3d triPlane(triangle.p0, triangle.p1, triangle.p2);
	if (triPlane.signedDistance(Vector3d(0.0, 0.0, 0.0)) < 0.0)
	{
		// Backface culling
		return;
	}
	
	Triangle3d screenTri;
	projectToScreen(screenTri, triangle);
	
	Box2i region;
	getRasterRegion(region, screenTri);
	raster(region, screenTri, triangle);
}

void Renderer::projectToScreen(Triangle3d& screenTri, const Triangle3d& triangle)
{
	Vector4d clip0 = mProjection * Vector4d(triangle.p0, 1.0);
	Vector4d clip1 = mProjection * Vector4d(triangle.p1, 1.0);
	Vector4d clip2 = mProjection * Vector4d(triangle.p2, 1.0);
	
	// TODO:: Clip to [-1.0, 1.0];
	
	// The coords are still homogeneous and needs to be mapped to screen space.
	clipToScreenSpace(screenTri.p0, clip0);
	clipToScreenSpace(screenTri.p1, clip1);
	clipToScreenSpace(screenTri.p2, clip2);
}

void Renderer::clipToScreenSpace(Vector3d& screen, const Vector4d& pt)
{
	double halfWidth = mVWidth / 2.0;
	double halfHeight = mVHeight / 2.0;
	double fn = (mDepthFar - mDepthNear) / 2.0;
	double nf = (mDepthNear - mDepthFar) / 2.0;
	
	// Normalized device coordinates
	Vector3d ndc = pt.xyz() / pt.w;
	
	// Window (or screen) coordinates
	// Viewport coordinates are (0, 0) bottom left corner
	// But our window/buffer coordinate system starts top left
	// Compensate for this as well.
	screen = Vector3d(ndc.x * halfWidth + (mVX + halfWidth), 
					  -ndc.y * halfHeight + (mVY + halfHeight),
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

template<typename T>
static inline T barycentricWeight(const Vector3d& bc, const T& c0, const T& c1, const T& c2)
{
	return c0 * bc.x + c1 * bc.y + c2 * bc.z;
}

void Renderer::raster(const Box2i& region, const Triangle3d& screenTri, const Triangle3d& triangle)
{
	const int minY = std::max(region.p0.y, 0);
	const int endY = std::min(region.p1.y + 1, mWindow->getHeight());
	const int minX = std::max(region.p0.x, 0);
	const int endX = std::min(region.p1.x + 1, mWindow->getWidth());
	
	ShaderInput shaderInput;
	shaderInput.lightContext = mLightContext;
	
	// For barycentric calculations
	double x01 = screenTri.p0.x - screenTri.p1.x;
	double x02 = screenTri.p0.x - screenTri.p2.x;
	double x10 = screenTri.p1.x - screenTri.p0.x;
	double x21 = screenTri.p2.x - screenTri.p1.x;
	
	double y01 = screenTri.p0.y - screenTri.p1.y;
	double y02 = screenTri.p0.y - screenTri.p2.y;
	double y10 = screenTri.p1.y - screenTri.p0.y;
	double y21 = screenTri.p2.y - screenTri.p1.y;
	
	double denom = x21 * y01 - x01 * y21;
	
	for (int y = minY; y < endY; y++)
	{
		for (int x = minX; x < endX; x++)
		{
			// Add a half, to adjust for the center of the pixel.
			// Screen coordinate (0, 0) is actually (0.5, 0.5)
			Vector2d coord((double)x + 0.5, (double)y + 0.5);
			
			// Barycentric coordinates
			Vector3d bc;
			bc.x = (x21 * (coord.y - screenTri.p1.y) - (coord.x - screenTri.p1.x) * y21) / denom;
			if (bc.x < 0.0 || bc.x > 1.0)
			{
				// Not inside triangle
				continue;
			}
			
			bc.y = (x02 * (coord.y - screenTri.p2.y) - (coord.x - screenTri.p2.x) * y02) / denom;
			if (bc.y < 0.0 || bc.y > 1.0)
			{
				// Not inside triangle
				continue;
			} 
			
			bc.z = 1.0 - bc.x - bc.y;
			if (bc.z < 0.0 || bc.z > 1.0)
			{
				// Not inside triangle
				continue;
			} 
			
			// Interpolate depth from barycentric coods.
			double depth = barycentricWeight(bc, screenTri.p0.z, screenTri.p1.z, screenTri.p2.z);
			
			// Depth check
			if (mDepthCheck && depth < mDepthBuffer[y][x])
			{
				continue;
			}
			
			// Fill depth buffer
			mDepthBuffer[y][x] = depth;
			
			// TODO: Project 3d-coord
			// TODO: Texture coord
			shaderInput.screenCoord = Vector3d(coord.x, coord.y, depth);
			shaderInput.vert = barycentricWeight(bc, triangle.p0, triangle.p1, triangle.p2);
			shaderInput.normal = barycentricWeight(bc, triangle.n0, triangle.n1, triangle.n2);
			shaderInput.normal.normalize();
			rasterPixel(shaderInput);
		}
	}
}

void Renderer::rasterPixel(ShaderInput& shaderInput)
{
	uint32_t color = mShader(shaderInput);
	// TODO: Blend func
	
	mWindow->putPixel(shaderInput.screenCoord.x, shaderInput.screenCoord.y, color);
}