#include <iostream>
#include <SDL/SDL.h>
#include "window.h"

namespace {

	const int xcWinWidth = 1024;
	const int xcWinHeight = 512;
	Window* xWindow = nullptr;

}

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	xWindow = new Window(xcWinWidth, xcWinHeight);
	xWindow->clear(0xFF0000);
	xWindow->blit();
	
	SDL_Delay(2000);
	
	return 0;
}