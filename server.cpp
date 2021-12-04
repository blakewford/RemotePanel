#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "remote.h"
#include "private.h"

static RemotePanel_ButtonState gButtonState;

#include <SDL.h>
static SDL_Window* w;
static int32_t SDL_Init(const RemotePanel_DisplayParams& params)
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

void RemotePanel_AttachControls(const char* ip, const RemotePanel_DisplayParams& params)
{
    if(params.data == nullptr) return;
    SDL_Init(params);
}

static volatile int gClientSocket = -1;
static volatile bool gKeepGoing = false;
void RemotePanel_PollControls()
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(CONTROL_PORT);
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

    SDL_Event e;
    SDL_WaitEventTimeout(&e, 8);
    if(e.type == SDL_QUIT)
    {
        SDL_Destroy();
        return;
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
    else if(e.type == SDL_KEYUP)
    {
        switch(e.key.keysym.sym)
        {
            case SDLK_UP:
                gButtonState.upButton = false;
                break;
            case SDLK_LEFT:
                gButtonState.leftButton = false;
                break;
            case SDLK_DOWN:
                gButtonState.downButton = false;
                break;
            case SDLK_RIGHT:
                gButtonState.rightButton = false;
                break;
            case SDLK_a:
                gButtonState.buttonA = false;
                break;
            case SDLK_b:
                gButtonState.buttonB = false;
                break;
        }
    }

    gClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    connect(gClientSocket, (struct sockaddr *)&address, sizeof(address));
    send(gClientSocket, &gButtonState, sizeof(RemotePanel_ButtonState), 0);
    close(gClientSocket);
    gClientSocket = -1;
}

void RemotePanel_DetachControls()
{
    gKeepGoing = false;
    SDL_Destroy();
}