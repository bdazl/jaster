#ifndef WINDOW_H
#define WINDOW_H

#include <cstdint>

struct SDL_Surface;

class Window
{
public:
	Window(int width, int height);
	~Window();
	
	void putPixel(int x, int y, int32_t color);
	
	void clear(int32_t color);
	void blit();
	
	int getWidth() { return mWidth; }
	int getHeight() { return mHeight; }
	
private:
	int mWidth, mHeight;
	SDL_Surface* mWinSurface;
	SDL_Surface* mRenderSurface;

};

#endif