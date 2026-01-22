#include "pch.h"
#include "ProSimBridge.h"
#include "ManagedWrapper.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace ProSimSDK;

// ============================================================================
// Error Message Storage
// ============================================================================

// Static storage for error messages (not thread-safe, but acceptable)
#pragma managed(push, off)
static char g_lastError[1024] = "";

static void SetLastError(const char* msg) {
    if (msg) {
        strncpy_s(g_lastError, sizeof(g_lastError), msg, _TRUNCATE);
    } else {
        g_lastError[0] = '\0';
    }
}
#pragma managed(pop)

// ============================================================================
// C API Implementation
// ============================================================================

extern "C" {

    void* ProSim_Create(void) {
        try {
            auto wrapper = new ProSimConnectWrapper();
            return static_cast<void*>(wrapper);
        }
        catch (Exception^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            return nullptr;
        }
        catch (...) {
            SetLastError("Unknown error creating ProSimConnect");
            return nullptr;
        }
    }

    BridgeResult ProSim_Connect(void* instance, const char* host, bool synchronous) {
        if (!instance) {
            SetLastError("Null instance handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }
        if (!host) {
            SetLastError("Null host string");
            return BRIDGE_ERR_INVALID_ARGUMENT;
        }

        try {
            auto wrapper = static_cast<ProSimConnectWrapper*>(instance);
            BridgeResult result = wrapper->Connect(host, synchronous);
            
            if (result == BRIDGE_OK) {
                // Clear any previous error
                SetLastError("");
            }
            
            return result;
        }
        catch (Exception^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            return BRIDGE_ERR_EXCEPTION;
        }
        catch (...) {
            SetLastError("Unknown error during connect");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    void ProSim_Disconnect(void* instance) {
        if (!instance) {
            return;
        }

        // Note: ProSimConnect doesn't have an explicit Disconnect method
        // The connection will be closed when the object is disposed via ProSim_Destroy
    }

    BridgeResult ProSim_IsConnected(void* instance, bool* out_connected) {
        if (!instance) {
            SetLastError("Null instance handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }
        if (!out_connected) {
            SetLastError("Null output pointer");
            return BRIDGE_ERR_INVALID_ARGUMENT;
        }

        try {
            auto wrapper = static_cast<ProSimConnectWrapper*>(instance);
            *out_connected = wrapper->IsConnected();
            
            // Clear any previous error
            SetLastError("");
            return BRIDGE_OK;
        }
        catch (Exception^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            *out_connected = false;
            return BRIDGE_ERR_EXCEPTION;
        }
        catch (...) {
            SetLastError("Unknown error checking connection");
            *out_connected = false;
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    void ProSim_Destroy(void* instance) {
        if (!instance) {
            return;
        }

        try {
            auto wrapper = static_cast<ProSimConnectWrapper*>(instance);
            delete wrapper;
        }
        catch (...) {
            // Ignore exceptions during cleanup
        }
    }

    BridgeResult ProSim_ReadDataRef(void* instance, const char* name, double* out_value) {
        if (!instance) {
            SetLastError("Null instance handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }
        if (!name) {
            SetLastError("Null DataRef name");
            return BRIDGE_ERR_INVALID_ARGUMENT;
        }
        if (!out_value) {
            SetLastError("Null output pointer");
            return BRIDGE_ERR_INVALID_ARGUMENT;
        }

        try {
            auto wrapper = static_cast<ProSimConnectWrapper*>(instance);
            
            if (!wrapper->IsConnected()) {
                SetLastError("Not connected to ProSim");
                *out_value = 0.0;
                return BRIDGE_ERR_NOT_CONNECTED;
            }

            ProSimConnect^ conn = wrapper->GetManagedConnection();
            Object^ value = conn->ReadDataRef(gcnew String(name));
            *out_value = Convert::ToDouble(value);
            
            // Clear any previous error
            SetLastError("");
            return BRIDGE_OK;
        }
        catch (DataRefNotFoundException^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            *out_value = 0.0;
            return BRIDGE_ERR_DATAREF_NOT_FOUND;
        }
        catch (NotConnectedException^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            *out_value = 0.0;
            return BRIDGE_ERR_NOT_CONNECTED;
        }
        catch (Exception^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            *out_value = 0.0;
            return BRIDGE_ERR_EXCEPTION;
        }
        catch (...) {
            SetLastError("Unknown error reading DataRef");
            *out_value = 0.0;
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    BridgeResult ProSim_WriteDataRef(void* instance, const char* name, double value) {
        if (!instance) {
            SetLastError("Null instance handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }
        if (!name) {
            SetLastError("Null DataRef name");
            return BRIDGE_ERR_INVALID_ARGUMENT;
        }

        try {
            auto wrapper = static_cast<ProSimConnectWrapper*>(instance);
            
            if (!wrapper->IsConnected()) {
                SetLastError("Not connected to ProSim");
                return BRIDGE_ERR_NOT_CONNECTED;
            }

            ProSimConnect^ conn = wrapper->GetManagedConnection();
            auto dataRef = gcnew DataRef(gcnew String(name), conn);
            dataRef->value = value;
            
            // Clear any previous error
            SetLastError("");
            return BRIDGE_OK;
        }
        catch (DataRefNotFoundException^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            return BRIDGE_ERR_DATAREF_NOT_FOUND;
        }
        catch (InvalidData^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            return BRIDGE_ERR_INVALID_DATA;
        }
        catch (NotConnectedException^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            return BRIDGE_ERR_NOT_CONNECTED;
        }
        catch (Exception^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            return BRIDGE_ERR_EXCEPTION;
        }
        catch (...) {
            SetLastError("Unknown error writing DataRef");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    const char* ProSim_GetLastError(void) {
        return g_lastError;
    }

    void ProSim_SetLastError(const char* msg) {
        SetLastError(msg);
    }

    // ============================================================================
    // DataRef Lifecycle Functions
    // ============================================================================

    DataRefHandle DataRef_Create(const char* name, int32_t interval, void* connection, bool register_now) {
        if (!name) {
            SetLastError("Null DataRef name");
            return nullptr;
        }
        if (!connection) {
            SetLastError("Null connection handle");
            return nullptr;
        }

        try {
            auto connWrapper = static_cast<ProSimConnectWrapper*>(connection);
            auto wrapper = new DataRefWrapper(name, interval, connWrapper, register_now);
            return static_cast<DataRefHandle>(wrapper);
        }
        catch (Exception^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            return nullptr;
        }
        catch (...) {
            SetLastError("Unknown error creating DataRef");
            return nullptr;
        }
    }

    void DataRef_Destroy(DataRefHandle handle) {
        if (!handle) {
            return;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            delete wrapper;
        }
        catch (...) {
            // Ignore exceptions during cleanup
        }
    }

    BridgeResult DataRef_Register(DataRefHandle handle) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->Register();
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (Exception^ ex) {
            String^ message = ex->Message;
            IntPtr ptr = Marshal::StringToHGlobalAnsi(message);
            try {
                SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
            return BRIDGE_ERR_EXCEPTION;
        }
        catch (...) {
            SetLastError("Unknown error registering DataRef");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    BridgeResult DataRef_GetName(DataRefHandle handle, char* out_buffer, int32_t buffer_size) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }
        if (!out_buffer || buffer_size <= 0) {
            SetLastError("Invalid buffer");
            return BRIDGE_ERR_INVALID_ARGUMENT;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            const char* name = wrapper->GetName();
            size_t len = strlen(name) + 1;

            if ((int32_t)len > buffer_size) {
                return (BridgeResult)len; // Return required size
            }

            strcpy_s(out_buffer, buffer_size, name);
            SetLastError("");
            return BRIDGE_OK;
        }
        catch (...) {
            SetLastError("Unknown error getting DataRef name");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    // ============================================================================
    // DataRef Type-Specific Getters
    // ============================================================================

    BridgeResult DataRef_GetInt(DataRefHandle handle, int32_t* out_value) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->GetInt(out_value);
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (...) {
            SetLastError("Unknown error getting int value");
            if (out_value) *out_value = 0;
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    BridgeResult DataRef_GetDouble(DataRefHandle handle, double* out_value) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->GetDouble(out_value);
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (...) {
            SetLastError("Unknown error getting double value");
            if (out_value) *out_value = 0.0;
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    BridgeResult DataRef_GetBool(DataRefHandle handle, bool* out_value) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->GetBool(out_value);
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (...) {
            SetLastError("Unknown error getting bool value");
            if (out_value) *out_value = false;
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    BridgeResult DataRef_GetString(DataRefHandle handle, char* out_buffer, int32_t buffer_size) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->GetString(out_buffer, buffer_size);
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (...) {
            SetLastError("Unknown error getting string value");
            if (out_buffer && buffer_size > 0) out_buffer[0] = '\0';
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    // ============================================================================
    // DataRef Type-Specific Setters
    // ============================================================================

    BridgeResult DataRef_SetInt(DataRefHandle handle, int32_t value) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->SetInt(value);
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (...) {
            SetLastError("Unknown error setting int value");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    BridgeResult DataRef_SetDouble(DataRefHandle handle, double value) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->SetDouble(value);
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (...) {
            SetLastError("Unknown error setting double value");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    BridgeResult DataRef_SetBool(DataRefHandle handle, bool value) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->SetBool(value);
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (...) {
            SetLastError("Unknown error setting bool value");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    BridgeResult DataRef_SetString(DataRefHandle handle, const char* value) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->SetString(value);
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (...) {
            SetLastError("Unknown error setting string value");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    // ============================================================================
    // Advanced DataRef Operations (Phase 5)
    // ============================================================================

    BridgeResult DataRef_GetDateTime(DataRefHandle handle, DateTime* out_value) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }
        if (!out_value) {
            SetLastError("Null output pointer");
            return BRIDGE_ERR_INVALID_ARGUMENT;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->GetDateTime(out_value);
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (...) {
            SetLastError("Unknown error getting DateTime value");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    BridgeResult DataRef_SetDateTime(DataRefHandle handle, const DateTime* value) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }
        if (!value) {
            SetLastError("Null DateTime pointer");
            return BRIDGE_ERR_INVALID_ARGUMENT;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->SetDateTime(value);
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (...) {
            SetLastError("Unknown error setting DateTime value");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    BridgeResult DataRef_SetReposition(DataRefHandle handle, const RepositionData* data) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }
        if (!data) {
            SetLastError("Null RepositionData pointer");
            return BRIDGE_ERR_INVALID_ARGUMENT;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            BridgeResult result = wrapper->SetReposition(data);
            
            if (result == BRIDGE_OK) {
                SetLastError("");
            }
            
            return result;
        }
        catch (...) {
            SetLastError("Unknown error setting RepositionData");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    // ============================================================================
    // Advanced Connection Operations (Phase 5)
    // ============================================================================

    BridgeResult ProSim_SetPriorityMode(void* instance, bool priority) {
        if (!instance) {
            SetLastError("Null instance handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<ProSimConnectWrapper*>(instance);
            wrapper->SetPriorityMode(priority);
            SetLastError("");
            return BRIDGE_OK;
        }
        catch (...) {
            SetLastError("Unknown error setting priority mode");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    // ============================================================================
    // Connection Callbacks
    // ============================================================================

    BridgeResult ProSim_SetOnConnect(void* instance, ConnectionCallback callback, void* user_data) {
        if (!instance) {
            SetLastError("Null instance handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<ProSimConnectWrapper*>(instance);
            wrapper->SetOnConnect(callback, user_data);
            SetLastError("");
            return BRIDGE_OK;
        }
        catch (...) {
            SetLastError("Unknown error setting connect callback");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    BridgeResult ProSim_SetOnDisconnect(void* instance, ConnectionCallback callback, void* user_data) {
        if (!instance) {
            SetLastError("Null instance handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<ProSimConnectWrapper*>(instance);
            wrapper->SetOnDisconnect(callback, user_data);
            SetLastError("");
            return BRIDGE_OK;
        }
        catch (...) {
            SetLastError("Unknown error setting disconnect callback");
            return BRIDGE_ERR_EXCEPTION;
        }
    }

    // ============================================================================
    // DataRef Callbacks
    // ============================================================================

    BridgeResult DataRef_SetOnDataChange(DataRefHandle handle, DataRefChangeCallback callback, void* user_data) {
        if (!handle) {
            SetLastError("Null DataRef handle");
            return BRIDGE_ERR_NULL_HANDLE;
        }

        try {
            auto wrapper = static_cast<DataRefWrapper*>(handle);
            wrapper->SetOnDataChange(callback, user_data);
            SetLastError("");
            return BRIDGE_OK;
        }
        catch (...) {
            SetLastError("Unknown error setting data change callback");
            return BRIDGE_ERR_EXCEPTION;
        }
    }
}