// affinecat
// by <njb@robotjunkyard.org>
//
// uses affine matrix transformation to blit a rotating picture of a cat
// around in a perspective that is reminiscient of Super NES Mode 7
//
// only external dependency is libSDL
//
// see Formula section at https://en.wikipedia.org/wiki/Mode_7 for details,
// and that application can be seen in drawBackground()

#ifdef __cplusplus
    #include <cstdlib>
#else
    #include <stdlib.h>
#endif

#include <SDL/SDL.h>
#include <math.h>
#include <stdio.h>

#include "cat.h"

#define UBYTE   unsigned char
#define U16     unsigned short

// framerate regulator snippet in main() is from:  http://lazyfoo.net/SDL_tutorials/lesson14/
const int framerate = 60;

inline int mod(int a, int b)
{
    const int r = a % b;
    return r < 0 ? r + b : r;
}

const int xres = 640,
          yres = 480,
          cxres = 256,   // window is 640x480 but we downscale-upscale image through a
          cyres = 244;   // lower SNES-like resolution to emulate that gritty-big-pixel look :D

// these get updated each frame by updateTransforms()
static float tca = 1.0, tcb = 0.0,
             tcc = 0.0, tcd = 1.0,
             tx0  = cxres/2, ty0 = cyres/2;
static int tick = 0;

// do this lin alg calculation for each pixel at (x, y)
//    [xi] =  [ a b x0 ] [ x - x0 ]
//    [yi]    [ c d y0 ] [ y - y0 ]
//                       [   1    ]
// therefore.....
//    xi = a(x - x0) + b(y - y0) + x0
//    yi = c(x - x0) + d(y - y0) + y1
void drawbackground(U16* screen, const U16* const tex)
{
    int horizon = 1;   // give this a try, too!:  (1 + 512*sin(tick*0.01) )
    for (int y = 0; y < cyres; y++)
    {
        for (int x = 0; x < cxres; x++)
        {
            int pz = y + horizon;

            short xi = ( (tca * (x - tx0)) + (tcb * (y - ty0)) + tx0) / pz;
            short yi = ( (tcc * (x - tx0)) + (tcd * (y - ty0)) + ty0) / pz;

            xi = mod(xi, 64);
            yi = mod(yi, 64);

            U16 color = ((tex)[(yi*64)+xi]);
            ((screen)[((y*cxres)+x)]) = color;
        }
    }
}

void blitScaled(const SDL_Surface* const src, SDL_Surface* dst)
{
    const int sw = src->w,
        sh = src->h,
        dw = dst->w,
        dh = dst->h;

    for (int dx = 0; dx < dw; dx++)
        for (int dy = 0; dy < dh; dy++)
        {
            const int sx = ((float)dx / (float)dw) * sw;
            const int sy = ((float)dy / (float)dh) * sh;

            ((U16*)(dst->pixels))[(dy*dw)+dx] = ((U16*)(src->pixels))[(sy*sw)+sx];;
        }
}

// inc frame and update dynamic transformations accordingly
void updateTransforms()
{
    tick++;
    const float rot    = (SDL_GetTicks() * 0.0005),
              scale    = 0.006,
             xscale    = scale,
             yscale    = scale;

    const float a11 = cos(rot)*(1/xscale),  a12 = -sin(rot)*(1/xscale),
                a21 = sin(rot)*(1/yscale),  a22 =  cos(rot)*(1/yscale);

    tca = a11, tcb = a12,
    tcc = a21, tcd = a22;

    tx0 = cxres / 2;
    ty0 = (cyres / 2);
}

int main ( int argc, char** argv )
{
    int timer = 0;

    // initialize SDL video
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    SDL_Surface* screen = SDL_SetVideoMode(xres, yres, 16,
                                           SDL_HWSURFACE
                                           /* | SDL_FULLSCREEN */
                                           | SDL_DOUBLEBUF
                                           );

    const SDL_PixelFormat& fmt = *(screen->format);
    SDL_Surface* canvas = SDL_CreateRGBSurface(SDL_HWSURFACE, cxres, cyres, 16,
        fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);

    if ( !screen )
    {
        printf("Unable to set video: %s\n", SDL_GetError());
        return 1;
    }

    // program main loop
    bool done = false;
    while (!done)
    {
        // message processing loop
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
            case SDL_QUIT:
                done = true;
                break;

                // check for keypresses
            case SDL_KEYDOWN:
                {
                    // exit if ESCAPE is pressed
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        done = true;
                    break;
                }
            }
        }

        // DRAWING STARTS HERE
        // clear screen
        SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

        updateTransforms();
        const void* const catpixels = &(cat_image.pixel_data);

        timer = SDL_GetTicks();
        drawbackground((U16*)canvas->pixels, (U16*)catpixels);
        blitScaled(canvas, screen);
        timer = SDL_GetTicks() - timer;
        // DRAWING ENDS HERE

        // finally, update the screen
        SDL_Flip(screen);

        if( ( timer < 1000 / framerate ) )
        {
            //Sleep the remaining frame time
            SDL_Delay( ( 1000 / framerate ) - timer );
        }
    } // end main loop

    printf("Bye.\n");
    return 0;
}
