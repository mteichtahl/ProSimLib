#pragma once

// Export for native C++
#ifdef PROSIMBRIDGE_EXPORTS
#define BRIDGE_API __declspec(dllexport)
#else
#define BRIDGE_API __declspec(dllimport)
#endif

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// Error Codes
// ============================================================================

typedef int32_t BridgeResult;

#define BRIDGE_OK                    0
#define BRIDGE_ERR_NULL_HANDLE      -1
#define BRIDGE_ERR_NOT_CONNECTED    -2
#define BRIDGE_ERR_CONNECTION_FAILED -3
#define BRIDGE_ERR_INVALID_ARGUMENT -4
#define BRIDGE_ERR_DATAREF_NOT_FOUND -5
#define BRIDGE_ERR_DATAREF_NOT_READY -6
#define BRIDGE_ERR_INVALID_DATA     -7
#define BRIDGE_ERR_EXCEPTION        -99

// ============================================================================
// Connection and Instance Management
// ============================================================================

extern "C" {
    // Creates a new ProSimConnect instance
    // Returns: Handle to the instance, or NULL on failure
    BRIDGE_API void* ProSim_Create(void);

    // Connects to ProSim at the specified host
    // host: null-terminated string (hostname or IP address, or NULL for localhost)
    // synchronous: if true, blocks until connected or fails
    // Returns: BRIDGE_OK on success, error code on failure
    BRIDGE_API BridgeResult ProSim_Connect(void* instance, const char* host, bool synchronous);

    // Disconnects from ProSim
    // instance: handle returned from ProSim_Create
    BRIDGE_API void ProSim_Disconnect(void* instance);

    // Checks if connected to ProSim
    // instance: handle returned from ProSim_Create
    // out_connected: pointer to receive connection status
    // Returns: BRIDGE_OK on success, error code on failure
    BRIDGE_API BridgeResult ProSim_IsConnected(void* instance, bool* out_connected);

    // Destroys a ProSimConnect instance and releases resources
    // instance: handle returned from ProSim_Create
    BRIDGE_API void ProSim_Destroy(void* instance);

    // ============================================================================
    // DataRef Operations (Legacy - for backward compatibility)
    // ============================================================================

    // Reads a DataRef value as a double
    // instance: handle returned from ProSim_Create
    // name: null-terminated string for the DataRef name
    // out_value: pointer to receive the value
    // Returns: BRIDGE_OK on success, error code on failure
    BRIDGE_API BridgeResult ProSim_ReadDataRef(void* instance, const char* name, double* out_value);

    // Writes a DataRef value from a double
    // instance: handle returned from ProSim_Create
    // name: null-terminated string for the DataRef name
    // value: the value to write
    // Returns: BRIDGE_OK on success, error code on failure
    BRIDGE_API BridgeResult ProSim_WriteDataRef(void* instance, const char* name, double value);

// ============================================================================
// Data Structures
// ============================================================================

    // DateTime structure for date/time DataRefs
    typedef struct {
        int32_t year;
        int32_t month;
        int32_t day;
        int32_t hour;
        int32_t minute;
        int32_t second;
        int32_t millisecond;
    } DateTime;

    // RepositionData structure for aircraft repositioning
    typedef struct {
        double latitude;
        double longitude;
        double altitude;
        double heading_magnetic;
        double pitch;
        double bank;
        double ias;
        bool on_ground;
    } RepositionData;

    // ============================================================================
    // Opaque Handle Types
    // ============================================================================

    // Opaque handle type for DataRef instances
    typedef void* DataRefHandle;

    // ============================================================================
    // Callback Function Pointer Types
    // ============================================================================

    // Connection callback - called when connection state changes
    // user_data: opaque pointer passed during registration
    typedef void (*ConnectionCallback)(void* user_data);

    // DataRef change callback - called when DataRef value changes
    // dataref_handle: handle to the DataRef that changed
    // user_data: opaque pointer passed during registration
    typedef void (*DataRefChangeCallback)(DataRefHandle dataref_handle, void* user_data);

    // ============================================================================
    // DataRef Lifecycle Management
    // ============================================================================

    // Creates a new DataRef instance
    // name: null-terminated string for the DataRef name
    // interval: polling interval in milliseconds
    // connection: handle returned from ProSim_Create
    // register_now: if true, registers immediately; if false, call DataRef_Register later
    // Returns: Handle to the DataRef, or NULL on failure
    BRIDGE_API DataRefHandle DataRef_Create(const char* name, int32_t interval, void* connection, bool register_now);

    // Destroys a DataRef instance and releases resources
    // handle: handle returned from DataRef_Create
    BRIDGE_API void DataRef_Destroy(DataRefHandle handle);

    // Registers the DataRef with ProSim
    // handle: handle returned from DataRef_Create
    // Returns: BRIDGE_OK on success, error code on failure
    BRIDGE_API BridgeResult DataRef_Register(DataRefHandle handle);

    // Gets the DataRef name
    // handle: handle returned from DataRef_Create
    // out_buffer: buffer to receive null-terminated string
    // buffer_size: size of buffer in bytes
    // Returns: BRIDGE_OK on success, required size if buffer too small, error code on failure
    BRIDGE_API BridgeResult DataRef_GetName(DataRefHandle handle, char* out_buffer, int32_t buffer_size);

    // ============================================================================
    // DataRef Type-Specific Getters
    // ============================================================================

    // Gets DataRef value as int32
    BRIDGE_API BridgeResult DataRef_GetInt(DataRefHandle handle, int32_t* out_value);

    // Gets DataRef value as double
    BRIDGE_API BridgeResult DataRef_GetDouble(DataRefHandle handle, double* out_value);

    // Gets DataRef value as bool
    BRIDGE_API BridgeResult DataRef_GetBool(DataRefHandle handle, bool* out_value);

    // Gets DataRef value as string
    BRIDGE_API BridgeResult DataRef_GetString(DataRefHandle handle, char* out_buffer, int32_t buffer_size);

    // ============================================================================
    // DataRef Type-Specific Setters
    // ============================================================================

    // Sets DataRef value from int32
    BRIDGE_API BridgeResult DataRef_SetInt(DataRefHandle handle, int32_t value);

    // Sets DataRef value from double
    BRIDGE_API BridgeResult DataRef_SetDouble(DataRefHandle handle, double value);

    // Sets DataRef value from bool
    BRIDGE_API BridgeResult DataRef_SetBool(DataRefHandle handle, bool value);

    // Sets DataRef value from string
    BRIDGE_API BridgeResult DataRef_SetString(DataRefHandle handle, const char* value);

    // ============================================================================
    // Advanced DataRef Operations (Phase 5)
    // ============================================================================

    // Gets DataRef value as DateTime
    BRIDGE_API BridgeResult DataRef_GetDateTime(DataRefHandle handle, DateTime* out_value);

    // Sets DataRef value from DateTime
    BRIDGE_API BridgeResult DataRef_SetDateTime(DataRefHandle handle, const DateTime* value);

    // Sets DataRef value for aircraft repositioning
    BRIDGE_API BridgeResult DataRef_SetReposition(DataRefHandle handle, const RepositionData* data);

    // ============================================================================
    // Advanced Connection Operations (Phase 5)
    // ============================================================================

    // Sets SDK priority mode for the connection
    // instance: handle returned from ProSim_Create
    // priority: if true, enables priority mode; if false, disables it
    // Returns: BRIDGE_OK on success, error code on failure
    BRIDGE_API BridgeResult ProSim_SetPriorityMode(void* instance, bool priority);

    // ============================================================================
    // Connection Callbacks
    // ============================================================================

    // Registers a callback for connection events
    // instance: handle returned from ProSim_Create
    // callback: function pointer to call when connected
    // user_data: opaque pointer passed to callback
    // Returns: BRIDGE_OK on success, error code on failure
    BRIDGE_API BridgeResult ProSim_SetOnConnect(void* instance, ConnectionCallback callback, void* user_data);

    // Registers a callback for disconnection events
    // instance: handle returned from ProSim_Create
    // callback: function pointer to call when disconnected
    // user_data: opaque pointer passed to callback
    // Returns: BRIDGE_OK on success, error code on failure
    BRIDGE_API BridgeResult ProSim_SetOnDisconnect(void* instance, ConnectionCallback callback, void* user_data);

    // ============================================================================
    // DataRef Callbacks
    // ============================================================================

    // Registers a callback for DataRef value changes
    // handle: handle returned from DataRef_Create
    // callback: function pointer to call when DataRef value changes
    // user_data: opaque pointer passed to callback
    // Returns: BRIDGE_OK on success, error code on failure
    BRIDGE_API BridgeResult DataRef_SetOnDataChange(DataRefHandle handle, DataRefChangeCallback callback, void* user_data);

    // ============================================================================
    // Error Handling
    // ============================================================================

    // Gets the last error message (thread-local)
    // Returns: null-terminated string describing the last error, or empty string if no error
    BRIDGE_API const char* ProSim_GetLastError(void);

    // Sets the last error message (for internal use)
    // msg: null-terminated error message string
    BRIDGE_API void ProSim_SetLastError(const char* msg);
}