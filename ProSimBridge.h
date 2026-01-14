// Export for native C++
#ifdef PROSIMBRIDGE_EXPORTS
#define BRIDGE_API __declspec(dllexport)
#else
#define BRIDGE_API __declspec(dllimport)
#endif

extern "C" {
    BRIDGE_API void* ProSim_Connect(const char* host);
    BRIDGE_API void ProSim_Disconnect(void* instance);
    BRIDGE_API bool ProSim_IsConnected(void* instance);
    BRIDGE_API double ProSim_ReadDataRef(void* instance, const char* name);
    BRIDGE_API void ProSim_WriteDataRef(void* instance, const char* name, double value);
    BRIDGE_API void ProSim_Destroy(void* instance);
}