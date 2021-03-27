#include <stdint.h>
#include <pthread.h>

#include "remote.h"
#include "private.h"

struct buttonState
{
    bool upButton = false;
    bool leftButton = false;
    bool downButton = false;
    bool rightButton = false;
    bool buttonA = false;
    bool buttonB = false;

    void clear()
    {
        upButton = false;
        leftButton = false;
        downButton = false;
        rightButton = false;
        buttonA = false;
        buttonB = false;
    }
};

static buttonState gButtonState;

//#define REMOTE

#ifndef REMOTE
#include <SDL.h>
static SDL_Window* w;
static int32_t SDL_Init(RemotePanel_DisplayParams params)
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0) return -1;
    w = SDL_CreateWindow("RemotePanel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, params.width, params.height, SDL_WINDOW_SHOWN);
    if(w == nullptr) return -1;
    return 0;
}

static void SDL_Destroy()
{
    SDL_DestroyWindow(w);
    SDL_Quit();
}

static volatile bool gKeepGoing = false;
static void* EventThread(void* buffer)
{
    gKeepGoing = true;
    while(gKeepGoing)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e) != 0)
        {
            if(e.type == SDL_QUIT)
            {
                SDL_Destroy();
                return nullptr;
            }
            else if(e.type == SDL_KEYDOWN)
            {
                switch(e.key.keysym.sym)
                {
                    case SDLK_UP:
                        gButtonState.upButton = true;
                        break;
                    case SDLK_LEFT:
                        gButtonState.leftButton = true;
                        break;
                    case SDLK_DOWN:
                        gButtonState.downButton = true;
                        break;
                    case SDLK_RIGHT:
                        gButtonState.rightButton = true;
                        break;
                    case SDLK_a:
                        gButtonState.buttonA = true;
                        break;
                    case SDLK_b:
                        gButtonState.buttonB = true;
                        break;
                }
            }
        }
    }
}
#endif

void RemotePanel_AttachControls(const char* ip)
{
#ifndef REMOTE
    RemotePanel_DisplayParams params = RemotePanel_GetDisplayParams();
    if(params.data == nullptr) return;

    SDL_Init(params);
    pthread_t thread;
    pthread_create(&thread, nullptr, EventThread, nullptr);
#endif
}

void RemotePanel_DetachControls()
{
#ifndef REMOTE
    gKeepGoing = false;
    SDL_Destroy();
#endif
}