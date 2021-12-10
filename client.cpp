#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <chrono>
#include <thread>
using namespace std::chrono;

#include "remote.h"
#include "private.h"

static volatile int gClientSocket = -1;
extern RemotePanel_DisplayParams gParams;

static void* client(void* ip) 
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(DISPLAY_PORT);
    inet_pton(AF_INET, (const char*)ip, &address.sin_addr);

    gClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    connect(gClientSocket, (struct sockaddr *)&address, sizeof(address));
    if(gClientSocket != -1) 
    {
        int32_t size = RemotePanel_GetBufferSize();
        bool keepGoing = send(gClientSocket, &gParams, sizeof(RemotePanel_DisplayParams), 0) == sizeof(RemotePanel_DisplayParams);
        while(keepGoing) 
        {
            int32_t wait = RemotePanel_GetFrameDelay();
            system_clock::time_point sync = system_clock::now() + milliseconds(wait);
            while(keepGoing)
            {
                keepGoing = send(gClientSocket, ((uint8_t*)gParams.data), size, MSG_WAITALL) == size;
            }
            while(system_clock::now() < sync)
            {
                std::this_thread::sleep_for(nanoseconds(1));
            }
        }
        free(gParams.data);
    }

    close(gClientSocket);

    return nullptr;
}

static volatile int gServerSocket = -1;
static volatile bool gKeepGoing = false;
static void* server(void*)
{
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(CONTROL_PORT);

    bind(gServerSocket, (struct sockaddr *) &address, sizeof(address));
    listen(gServerSocket, 1);

    RemotePanel_ButtonState state;
    while(gKeepGoing)
    {
        int sock = accept(gServerSocket, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (sock != -1)
        {
            bool success = read(sock, &state, sizeof(RemotePanel_ButtonState)) == sizeof(RemotePanel_ButtonState);
            if(success && gKeepGoing)
            {
            }
            close(sock);
        }
    }

    return nullptr;
}

static pthread_t gClientThread;
static pthread_t gServerThread;
void RemotePanel_StartClient(const char* ip)
{
    pthread_create(&gClientThread, nullptr, client, (void*)ip);
    gServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    gKeepGoing = pthread_create(&gServerThread, nullptr, server, nullptr) == 0;
}

void RemotePanel_WriteDisplayBuffer(void* data, int32_t size)
{
    memcpy(gParams.data, data, size);
}

void RemotePanel_StopClient()
{
    gKeepGoing = false;
    shutdown(gServerSocket, SHUT_RD); // Break accept
    pthread_join(gServerThread, nullptr);
    close(gServerSocket);

    close(gClientSocket);
    pthread_join(gClientThread, nullptr);
}