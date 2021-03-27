#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "remote.h"
#include "private.h"

static pthread_t gServerThread;
static volatile uint64_t gFrames = 0;
static volatile int gServerSocket = -1;
static volatile bool gKeepGoing = false;
static void dumpToFile(void* data, int32_t width, int32_t height, int32_t size)
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
    while(size)
    {
        fwrite(data + cursor, 3, 1, test);
        size-=4;
        cursor+=4;
    }
    fclose(test);
}

static void* server(void*) 
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
        bool success = read(sock, &params, sizeof(RemotePanel_DisplayParams)) == sizeof(RemotePanel_DisplayParams);
        RemotePanel_SetDisplayParams(params);
        RemotePanel_AttachControls("127.0.0.1");

        int32_t size = 0;
        switch(params.type)
        {
            case LOW_RES:
                size = sizeof(uint16_t)*params.width*params.height;
                break;
            case HI_RES:
                size = sizeof(uint32_t)*params.width*params.height;
                break;
        }
        params.data = malloc(size);
        while(gKeepGoing) 
        {
            int32_t count = 0;
            while(success && count < (size/PACKET_SIZE))
            {
                success = read(sock, params.data + (count*PACKET_SIZE), PACKET_SIZE) == PACKET_SIZE;
                count++;
            }
            if(success && gKeepGoing)
            {
                dumpToFile(params.data, params.width, params.height, size);
                gFrames++;
            }

            gKeepGoing = success;
        }

        RemotePanel_DetachControls();
        free(params.data);
        close(sock);
    }

    return nullptr;
}

#include <cstring>
void demo()
{
    RemotePanel_DisplayParams params;
    params.width = 128;
    params.height = 64;
    params.type = HI_RES;
    RemotePanel_SetDisplayParams(params);
    RemotePanel_SetMaxFramesPerSecond(1);

    const int32_t size = RemotePanel_GetBufferSize();

    char buffer[size];
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
    RemotePanel_WriteDisplayBuffer(buffer, size);
    RemotePanel_StartClient("127.0.0.1");
    RemotePanel_AttachControls("127.0.0.1");

    usleep(1000*1000*5);

    RemotePanel_DetachControls();
    RemotePanel_StopClient();
}

int main(int argc, char** argv)
{
    gServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    gKeepGoing = pthread_create(&gServerThread, nullptr, server, nullptr) == 0;

    if(argc > 1 && !strcmp("demo", argv[1]))
    {
        demo();
    }
    else
    {
        while(gKeepGoing)
        {
            usleep(1);
        }
    }

    shutdown(gServerSocket, SHUT_RD); // Break accept
    pthread_join(gServerThread, nullptr);
    close(gServerSocket);

    return 0;
}
