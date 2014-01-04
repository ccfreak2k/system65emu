#include "SDLContext.hpp"

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

SDLContext::SDLContext() :
	m_SDLRenderer(NULL),
	m_SDLWindow(NULL),
	m_FontSurface(NULL),
	m_FontTexture(NULL)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		throw SDLContextException;
}

SDLContext::~SDLContext()
{
	if (m_FontTexture != NULL)
		SDL_DestroyTexture(m_FontTexture);

	if (m_FontSurface != NULL)
		SDL_FreeSurface(m_FontSurface);

	if (m_SDLRenderer != NULL)
		SDL_DestroyRenderer(m_SDLRenderer);

	if (m_SDLWindow != NULL)
		SDL_DestroyWindow(m_SDLWindow);

	SDL_Quit();
}

void SDLContext::InitVideo(int width = 640, int height = 480)
{
	if (SDL_CreateWindowAndRenderer(width,height,0,&m_SDLWindow,&m_SDLRenderer) || m_SDLWindow == NULL || m_SDLRenderer == NULL)
		throw SDLContextException;

	SDL_SetWindowTitle(m_SDLWindow, m_szWindowTitle);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(m_SDLRenderer, width, height);
}

void SDLContext::LoadFont(const char *filename = "font.bmp")
{
    m_FontSurface = SDL_LoadBMP(filename);
    if (m_FontSurface == NULL)
        throw SDLContextException;

	m_FontTexture = SDL_CreateTextureFromSurface(m_SDLRenderer,m_FontSurface);
	if (m_FontTexture == NULL) {
		SDL_FreeSurface(m_FontSurface);
		throw SDLContextException;
	}
}
