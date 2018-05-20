#include "renderer.h"
#include "Window.h"
#include "geometry/plane.h"
#include "geometry/frustum.h"
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
			Vector3d dir = light.pos - input.vert;
			dir.normalize();
			
			Vector3d toEye = -input.vert;
			toEye.normalize();
			
			Vector3d reflect = -math::reflect(dir, input.normal);
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
	mViewport(std::make_shared<Viewport>(window->getWidth(), window->getHeight())),
	mDepthCheck(true),
	mDepthBuffer()
{
	mCamera = std::make_shared<Frustum>(2.0 * atan(mWindow->getHeight() / 2.0 / xcNear), mWindow->getWidth() / (double)mWindow->getHeight(), xcNear, xcFar);
	
	for (int y = 0; y < window->getHeight(); y++)
	{
		mDepthBuffer.push_back(std::vector<double>());
		for (int x = 0; x < window->getWidth(); x++)
		{
			mDepthBuffer[y].push_back(-mViewport->getDepthFar());
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

void Renderer::clearDepthBuffer()
{
	double clr = -mViewport->getDepthFar();
	for (auto& it : mDepthBuffer)
	{
		std::for_each(it.begin(), it.end(), [clr](double& d){ d = clr; });
	}
}

void Renderer::renderTriangle(const Triangle3d& triangle)
{
	Plane triPlane(triangle.p0, triangle.p1, triangle.p2);
	if (triPlane.signedDistance(mCamera->getTransform().getPosition()) < 0.0)
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
	Vector3d p0 = mCamera->project(triangle.p0);
	p0.y = -p0.y; // Flip sign to get top left corner = [0, 0]
	screenTri.p0 = mCamera->ndcToViewportSpace(p0, *mViewport);
	
	Vector3d p1 = mCamera->project(triangle.p1);
	p1.y = -p1.y;
	screenTri.p1 = mCamera->ndcToViewportSpace(p1, *mViewport);
	
	Vector3d p2 = mCamera->project(triangle.p2);
	p2.y = -p2.y;
	screenTri.p2 = mCamera->ndcToViewportSpace(p2, *mViewport);
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