#ifndef __AVR__
#include <cmath>
#include <cstdlib>
#include "remote.h"
#endif

RemotePanel_DisplayParams gParams;
static volatile uint8_t gFPS = 60;

extern "C"
{
    void RemotePanel_SetDisplayParams(RemotePanel_DisplayParams params)
    {
        gParams.width = params.width;
        gParams.height = params.height;
        gParams.type = params.type;
#ifndef __AVR__
        gParams.data = malloc(RemotePanel_GetBufferSize());
#endif
    }

    int32_t RemotePanel_GetBufferSize()
    {
        int32_t size = 0;
        switch(gParams.type)
        {
            case LOW_RES:
                size = sizeof(uint16_t)*gParams.width*gParams.height;
                break;
            case HI_RES:
                size = sizeof(uint32_t)*gParams.width*gParams.height;
                break;
        }
        return size;
    }    

    void RemotePanel_SetMaxFramesPerSecond(uint8_t frames)
    {
        gFPS = frames;
    }

    int32_t RemotePanel_GetFrameDelay()
    {
        return floor((1000*1000*1)/gFPS);
    }    
}