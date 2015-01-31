#include <iostream>
#include <SDL/SDL.h>
#include "window.h"
#include "Renderer.h"
#include "tiny_obj_loader.h"

namespace {

	const int xcWinWidth = 1024;
	const int xcWinHeight = 512;
	std::shared_ptr<Window> xWindow = nullptr;
	std::shared_ptr<Renderer> xRenderer = nullptr;
	
	std::vector<Triangle3d> dragonMesh;
	
	void loadObjFile(std::vector<Triangle3d>& mesh, const std::string& file)
	{
		std::cout << "Loading \"" << file << "..." << std::endl;
		
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err = tinyobj::LoadObj(shapes, materials, file.c_str());
		
		if (!err.empty())
		{
			std::cerr << "Failure." << std::endl;
			std::cerr << "Error: " << err << std::endl;
			exit(1);
		}
		
		std::cout << "-> shapes    : " << shapes.size() << std::endl;
		std::cout << "-> materials : " << materials.size() << std::endl;
		
		std::cout << "Converting triangles..." << std::endl;
		
		for (auto shape : shapes)
		{
			Triangle3d tri;
			for (int i = 0; i < shape.mesh.indices.size() / 3; i++)
			{
				int idx = shape.mesh.indices[3*i+0];
				tri.p0 = Vector3d((double)shape.mesh.positions[3*idx+0],
								  (double)shape.mesh.positions[3*idx+1],
								  (double)shape.mesh.positions[3*idx+2]);
			
				idx = shape.mesh.indices[3*i+1];
				tri.p1 = Vector3d((double)shape.mesh.positions[3*idx+0],
								  (double)shape.mesh.positions[3*idx+1],
								  (double)shape.mesh.positions[3*idx+2]);
								  
				idx = shape.mesh.indices[3*i+2];
				tri.p1 = Vector3d((double)shape.mesh.positions[3*idx+0],
				  				  (double)shape.mesh.positions[3*idx+1],
				  				  (double)shape.mesh.positions[3*idx+2]);
								  
				mesh.push_back(tri);
			}
		}
		
		std::cout << "-> triangles: " << mesh.size() << std::endl;
		std::cout << "Success!" << std::endl;
	}
	
	void renderMesh(const std::vector<Triangle3d>& mesh, const Matrix4d& transform)
	{
		Triangle3d global;
		for (auto tri : mesh)
		{
			math::transform(global, transform, tri);
			xRenderer->renderTriangle(global);
		}
	}

}

int main(int argc, char** argv)
{
	loadObjFile(dragonMesh, "obj/cow.obj");
	
	std::cout << "Initializing SDL..." << std::endl;
	SDL_Init(SDL_INIT_EVERYTHING);
	
	std::cout << "Initializing renderer..." << std::endl;
	xWindow = std::make_shared<Window>(xcWinWidth, xcWinHeight);
	xRenderer = std::make_shared<Renderer>(xWindow);
	
	Matrix4d scale = Matrix4d::createScale(0.5, 0.5, 0.5);
	Matrix4d translate = Matrix4d::createTranslation(0.0, 0.0, -5.0);
	Matrix4d rotationStep = Matrix4d::createRotationAroundAxis(0.0, 180.0 / 25, 0.0);
	Matrix4d rotation, transform;
	
	SDL_Event event;
	bool quit = false;
	bool changes = true;
	while(!quit)
	{
		if (changes)
		{
			//std::cout << "DBG: Rendering..." << std::endl;
			xWindow->clear(0x000AFF);
		
			rotation = rotationStep * rotation;
			transform = translate * rotation * scale;
		
			renderMesh(dragonMesh, transform);
		
			xWindow->blit();
			
			//changes = false;
		}
		
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