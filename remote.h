extern "C"
{
    enum RemotePanel_Resolution: uint8_t
    {
        LOW_RES,
        HI_RES
    };

    #pragma pack(push, 1)
    struct RemotePanel_DisplayParams
    {
        uint16_t width = 0;
        uint16_t height = 0;
        uint8_t type = HI_RES;
        void* data = nullptr;
    };
    #pragma pack(pop)

    #pragma pack(push, 1)
    struct RemotePanel_ButtonState
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
    #pragma pack(pop)

    void RemotePanel_SetDisplayParams(RemotePanel_DisplayParams params);
    void RemotePanel_SetMaxFramesPerSecond(uint8_t frames);
    int32_t RemotePanel_GetFrameDelay();
    int32_t RemotePanel_GetBufferSize();
    void RemotePanel_StartClient(const char* ip);
    void RemotePanel_AttachControls(const char* ip, const RemotePanel_DisplayParams& params);
    void RemotePanel_WriteDisplayBuffer(void* data, int32_t size);
    void RemotePanel_PollControls();
    void RemotePanel_DetachControls();
    void RemotePanel_StopClient();
}
