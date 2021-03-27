extern "C"
{
    #pragma pack(push, 1)
    struct RemotePanel_DisplayParams
    {
        uint16_t width;
        uint16_t height;
        uint8_t type;
        void* data;
    };
    #pragma pack(pop)

    void RemotePanel_SetDisplayParams(RemotePanel_DisplayParams params);
    void RemotePanel_SetMaxFramesPerSecond(uint8_t frames);
    int32_t RemotePanel_GetBufferSize();
    void RemotePanel_StartClient(const char* ip);
    void RemotePanel_WriteDisplayBuffer(void* data, int32_t size);
    void RemotePanel_StopClient();

    enum RemotePanel_Resolution: uint8_t
    {
        LOW_RES,
        HI_RES
    };
}
