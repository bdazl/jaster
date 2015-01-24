#include "window.h"
#include <SDL/SDL.h>

namespace{

	void putPixel24_nolock(SDL_Surface * surface, int x, int y, int32_t color)
	{
		Uint8 * pixel = (Uint8*)surface->pixels;
		pixel += (y * surface->pitch) + (x * sizeof(Uint8) * 3);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		pixel[0] = (color >> 24) & 0xFF;
		pixel[1] = (color >> 16) & 0xFF;
		pixel[2] = (color >> 8) & 0xFF;
#else
		pixel[0] = color & 0xFF;
		pixel[1] = (color >> 8) & 0xFF;
		pixel[2] = (color >> 16) & 0xFF;
#endif
	}

	void putPixel24(SDL_Surface * surface, int x, int y, int32_t color)
	{
		if( SDL_MUSTLOCK(surface) )
		{
			SDL_LockSurface(surface);
		}
		
		putPixel24_nolock(surface, x, y, color);
		
		if( SDL_MUSTLOCK(surface) )
		{
			SDL_LockSurface(surface);
		}
	}

	SDL_Surface* createSurface(int32_t flags, int width, int height, const SDL_Surface* display)
	{
		// 'display' is the surface whose format you want to match
		//  if this is really the display format, then use the surface returned from SDL_SetVideoMode

		const SDL_PixelFormat& fmt = *(display->format);
		return SDL_CreateRGBSurface(flags, width, height,
									fmt.BitsPerPixel,
									fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask );
	}
}

Window::Window(int width, int height) :
	mWidth(width),
	mHeight(height),
	mWinSurface(nullptr),
	mRenderSurface(nullptr)
{
	mWinSurface = SDL_SetVideoMode(mWidth, mHeight, 24, SDL_SWSURFACE);
	mRenderSurface = createSurface(SDL_SWSURFACE, mWidth, mHeight, mWinSurface);
}

Window::~Window()
{
	if (mRenderSurface)
	{
		SDL_FreeSurface(mRenderSurface);
	}
	
	if (mWinSurface)
	{
		SDL_FreeSurface(mWinSurface);
	}
}

void Window::putPixel(int x, int y, int32_t color)
{
	putPixel24(mRenderSurface, x, y, color);
}

void Window::clear(int32_t color)
{
	for (int y = 0; y < mHeight; y++)
	{
		for (int x = 0; x < mWidth; x++)
		{
			putPixel(x, y, color);
		}
	}
}

void Window::blit()
{
	SDL_BlitSurface(mRenderSurface, nullptr, mWinSurface, nullptr);
	SDL_Flip(mWinSurface);
}