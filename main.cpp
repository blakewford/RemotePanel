#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "remote.h"
#include "private.h"

static pthread_t gClientThread;
static volatile uint64_t gFrames = 0;
static volatile int gServerSocket = -1;
static volatile bool gKeepGoing = true;
static void dumpToFile(void* data, int32_t width, int32_t height, int32_t size, uint8_t resolution)
{
    uint8_t header[54];
    header[0] = 'B';
    header[1] = 'M';

    int32_t imageSize = (size*3)/4;
    *(uint32_t*)(&header[2]) = (uint32_t)(imageSize + 54);
    *(uint32_t*)(&header[6]) = (uint32_t)0;
    *(uint32_t*)(&header[10]) = (uint32_t)54;
    *(uint32_t*)(&header[14]) = (uint32_t)40;
    *(uint32_t*)(&header[18]) = (int32_t)width;
    *(uint32_t*)(&header[22]) = (int32_t)-height;
    *(uint16_t*)(&header[26]) = (uint16_t)1;
    *(uint16_t*)(&header[28]) = (uint16_t)24;
    *(uint32_t*)(&header[30]) = (uint32_t)0;
    *(uint32_t*)(&header[34]) = (uint32_t)imageSize;
    *(uint32_t*)(&header[38]) = (uint32_t)2835;
    *(uint32_t*)(&header[42]) = (uint32_t)2835;
    *(uint32_t*)(&header[46]) = (uint32_t)0;
    *(uint32_t*)(&header[50]) = (uint32_t)0;

    int32_t cursor = 0;
    FILE* test = fopen("test.bmp", "w");
    fwrite(header, 54, 1, test);
    if(resolution == HI_RES)
    {
        while(size)
        {
            fwrite(((uint8_t*)data) + cursor, 3, 1, test);
            size-=4;
            cursor+=4;
        }
    }
    else if(resolution == LOW_RES)
    {
        while(size)
        {
            uint16_t compressed = *((uint16_t*)(((uint8_t*)data) + cursor));
            uint8_t r = ((compressed & 0xF800) >> 11) * 8;
            uint8_t g = ((compressed & 0x07E0) >> 5) * 4;
            uint8_t b = (compressed & 0x001F) * 8;
            fwrite(&b, 1, 1, test);
            fwrite(&g, 1, 1, test);
            fwrite(&r, 1, 1, test);
            size-=2;
            cursor+=2;
        }
    }
    else if(resolution == BINARY)
    {
        while(size)
        {
            uint8_t j = 8;
            uint8_t compressed = ((uint8_t*)data)[cursor];
            while(j--)
            {
                uint32_t pixel = ((compressed >> j) & 0x1) == 0? 0: ~0;
                fwrite(&pixel, 3, 1, test);
            }
            size--;
            cursor++;
        }
    }

    fclose(test);
}

static void server()
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(DISPLAY_PORT);

    bind(gServerSocket, (struct sockaddr *) &address, sizeof(address));
    listen(gServerSocket, 1);

    int sock = accept(gServerSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (sock != -1) 
    {
        RemotePanel_DisplayParams params;
        bool success = recv(sock, &params, sizeof(RemotePanel_DisplayParams), MSG_WAITALL) == sizeof(RemotePanel_DisplayParams);
        RemotePanel_AttachControls("127.0.0.1", params);

        int32_t size = 0;
        switch(params.type)
        {
            case LOW_RES:
                size = sizeof(uint16_t)*params.width*params.height;
                break;
            case HI_RES:
                size = sizeof(uint32_t)*params.width*params.height;
                break;
            case BINARY:
                size = (params.width*params.height)/8;
                break;
        }

        params.data = malloc(size);
        while(gKeepGoing)
        {
            int32_t total = 0;
            while(success && (total < size))
            {
                int count = recv(sock, ((uint8_t*)params.data) + total, size-total, 0);
                success = count > 0;
                total+=count;
            }
            if(success && gKeepGoing)
            {
                dumpToFile(params.data, params.width, params.height, size, params.type);
                RemotePanel_PollControls();
                gFrames++;
            }
            gKeepGoing = success;
        }

        RemotePanel_DetachControls();
        free(params.data);
        close(sock);
    }
}

int32_t demoSetup(uint8_t type)
{
    RemotePanel_DisplayParams params;
    params.width = 128;
    params.height = 64;
    params.type = type;

    RemotePanel_SetDisplayParams(params);
    RemotePanel_SetMaxFramesPerSecond(1);

    return RemotePanel_GetBufferSize();;
}

void buildHighResolutionBuffer(uint8_t* buffer, int32_t size)
{
    int32_t cursor = 0;
    int32_t toWrite = size;
    while(toWrite)
    {
        memset(buffer + cursor, 0x00, 1);
        memset(buffer + cursor + 1, 0x00, 1);
        memset(buffer + cursor + 2, 0xFF, 1);
        toWrite-=4;
        cursor+=4;
    }
}

void buildLowResolutionBuffer(uint8_t* buffer, int32_t size)
{
    int32_t cursor = 0;
    int32_t toWrite = size;
    while(toWrite)
    {
        memset(buffer + cursor, 0x00, 1);
        memset(buffer + cursor + 1, 0xF8, 1);
        toWrite-=2;
        cursor+=2;
    }
}

void buildBinaryResolutionBuffer(uint8_t* buffer, int32_t size)
{
    memset(buffer, 0xFF, size);
}

void* demo(void* p)
{
    uint8_t type = *((uint8_t*)p);
    const int32_t size = demoSetup(type);
    uint8_t buffer[size];

    switch(type)
    {
        case HI_RES:
            buildHighResolutionBuffer(buffer, size);
            break;
        case LOW_RES:
            buildLowResolutionBuffer(buffer, size);
            break;
        case BINARY:
            buildBinaryResolutionBuffer(buffer, size);
            break;
    }

    RemotePanel_WriteDisplayBuffer(buffer, size);
    RemotePanel_StartClient("127.0.0.1");

    usleep(1000*1000*5);

    RemotePanel_StopClient();

    return nullptr;
}

int main(int argc, char** argv)
{
    uint8_t type = LOW_RES;
    gServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(argc > 1 && !strcmp("demo", argv[1]))
    {
        gKeepGoing = pthread_create(&gClientThread, nullptr, demo, &type) == 0;
    }

    server();

    shutdown(gServerSocket, SHUT_RD); // Break accept
    pthread_join(gClientThread, nullptr);
    close(gServerSocket);

    return 0;
}
