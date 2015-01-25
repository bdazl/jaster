#include <iostream>
#include <SDL/SDL.h>
#include "window.h"
#include "Renderer.h"

namespace {

	const int xcWinWidth = 1024;
	const int xcWinHeight = 512;
	std::shared_ptr<Window> xWindow = nullptr;

}

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	xWindow = std::make_shared<Window>(xcWinWidth, xcWinHeight);
	
	Renderer renderer(xWindow);
	
	Triangle3d originalTri, globalTri;
	originalTri.p0 = Vector3d(-1.0, 0.0, 0.0);
	originalTri.p1 = Vector3d(1.0, 0.0, 0.0);
	originalTri.p2 = Vector3d(1.0, 1.0, 0.0);
	
	Matrix4d translate = Matrix4d::createTranslation(0.0, 0.0, -3.0);
	Matrix4d rotationStep = Matrix4d::createRotationAroundAxis(0.0, 360.0/50, 0.0);
	Matrix4d rotation, transform;
	
	SDL_Event event;
	bool quit = false;
	while(!quit)
	{
		xWindow->clear(0x000AFF);
		
		rotation = rotationStep * rotation;
		transform = translate * rotation;
		math::transform(globalTri, transform, originalTri);
	
		renderer.renderTriangle(globalTri);
		
		xWindow->blit();
		
		while(SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				quit = true;
				break;
			}
		}
		
		if (!quit)
		{
			SDL_Delay(20);
		}
	}
	
	return 0;
}