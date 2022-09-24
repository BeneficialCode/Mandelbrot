// Mandelbrot.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <complex>
#include <cassert>
#include "SDL.h"

using namespace std;

class MandelPixel {
public:
    MandelPixel(SDL_Surface* screen, int x, int y, int xdim, int ydim, int maxIter)
        :_screen(screen), _maxIter(maxIter), _iter(0), _c(x, y)
    {
        // scale y to [-1.2,1.2] and shift -0.5+0i to the center
        _c *= 2.4f / static_cast<float>(ydim);
        _c -= complex<float>(1.2 * xdim / ydim + 0.5, 1.2);
        Iterate();
    }

    int Iterations() const { return _iter; }
    uint32_t Color() const
    {
        if (_iter == _maxIter) return SDL_MapRGB(_screen->format, 0, 0, 0);
        const int ci = 512 * _iter / _maxIter; // incremental value for color
        // return SDL_MapRGB(screen->format, 40*iter & 255, 66*iter & 255, 100*iter & 255);
        return _iter < _maxIter / 2 ? SDL_MapRGB(_screen->format, ci, 0, 0)
            : SDL_MapRGB(_screen->format, 255, ci - 255, ci - 255);
    }
private:
    void Iterate() {
        complex<float> z = _c;
        for (; _iter < _maxIter && norm(z) <= 4.0f; _iter++)
            z = z * z + _c;
    }

    SDL_Surface* _screen;
    const int _maxIter;
    int _iter;
    complex<float> _c;
};

void putPixel(SDL_Surface* screen, int x, int y, uint32_t pixel) {
    assert(screen->format->BytesPerPixel == 4);
    auto pixel_address = reinterpret_cast<uint32_t*>(screen->pixels) + y * screen->w + x;
    *pixel_address = pixel;
}

int main(int argc,char* argv[]){
    const int maxIter = 30;
    int xdim = 1200, ydim = 800;
    if (argc >= 3)
        xdim = atoi(argv[1]), ydim = atoi(argv[2]);

    if (SDL_Init(SDL_INIT_VIDEO) == -1)
        return -1;

    SDL_Window* window = SDL_CreateWindow("Mandelbrot Set", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, xdim, ydim, SDL_WINDOW_SHOWN);
    if (!window)
        return -1;

    SDL_Surface* screen = SDL_GetWindowSurface(window);
    if (!screen)
        return -1;

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer)
        return -1;

    SDL_LockSurface(screen);
    for(int y=0;y< ydim;y++)
        for (int x = 0; x < xdim; x++) {
            MandelPixel m(screen, x, y, xdim, ydim, maxIter);
            putPixel(screen, x, y, m.Color());
        }
    SDL_UnlockSurface(screen);

    SDL_RenderPresent(renderer);
    // save the fractal
    SDL_SaveBMP(screen, "mandelbrot.bmp");

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, screen);
    if (!texture)
        return -1;

    SDL_Rect rect;
    rect.w = xdim;
    rect.h = ydim;
    rect.x = (screen->w - rect.w) / 2;
    rect.y = (screen->h - rect.h) / 2;

    SDL_Point leftPPoint;

    bool run = true;
    while (run) {
        // Polling for events as long as there exists some
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: run = false; break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LMASK)
                    {
                        leftPPoint.x = event.button.x;
                        leftPPoint.y = event.button.y;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (event.button.button == SDL_BUTTON_LMASK)
                    {
                        SDL_Point tLeftPPoint;
                        tLeftPPoint.x = event.button.x;
                        tLeftPPoint.y = event.button.y;

                        rect.x += tLeftPPoint.x - leftPPoint.x;
                        rect.y += tLeftPPoint.y - leftPPoint.y;

                        leftPPoint = tLeftPPoint;
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    if (event.wheel.y > 0)
                    {
                        rect.w *= 1.1;
                        rect.h *= 1.1;
                        rect.x = (screen->w - rect.w) / 2;
                        rect.y = (screen->h - rect.h) / 2;
                    }
                    if (event.wheel.y < 0)
                    {
                        rect.w /= 1.1;
                        rect.h /= 1.1;
                        rect.x = (screen->w - rect.w) / 2;
                        rect.y = (screen->h - rect.h) / 2;
                    }
                    break;
            }
        }

        if (SDL_RenderClear(renderer) != 0) { return -1; }
        if (SDL_RenderCopy(renderer, texture, nullptr, &rect) != 0) { return -1; }
        SDL_RenderPresent(renderer);


    }
    SDL_Quit();

    return 0;
}


