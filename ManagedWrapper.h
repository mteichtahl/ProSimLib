// ManagedWrapper.h
// Native C++ classes that wrap managed .NET objects using gcroot<T>
// These classes bridge between the C API and the managed ProSimSDK types

#pragma once

#include <vcclr.h>
#include <msclr/gcroot.h>
#include "ProSimBridge.h"

// Forward declarations
class DataRefWrapper;
class ProSimConnectWrapper;

// ============================================================================
// Ref class to bridge native callbacks to managed delegates
// ============================================================================

ref class ConnectionEventBridge {
private:
    ProSimConnectWrapper* _nativeWrapper;

public:
    ConnectionEventBridge(ProSimConnectWrapper* wrapper) : _nativeWrapper(wrapper) {}

    void OnConnect();
    void OnDisconnect();
};

// ============================================================================
// Ref class to bridge native callbacks to managed DataRef delegates
// ============================================================================

ref class DataRefEventBridge {
private:
    DataRefWrapper* _nativeWrapper;

public:
    DataRefEventBridge(DataRefWrapper* wrapper) : _nativeWrapper(wrapper) {}

    void OnDataChange(ProSimSDK::DataRef^ dataRef);
};

// ============================================================================
// ProSimConnectWrapper
// Native class that wraps a managed ProSimConnect instance
// ============================================================================

class ProSimConnectWrapper {
private:
    msclr::gcroot<ProSimSDK::ProSimConnect^> _connection;
    msclr::gcroot<ConnectionEventBridge^> _eventBridge;

    // Native callback storage
    ConnectionCallback _onConnectCallback;
    void* _onConnectUserData;
    ConnectionCallback _onDisconnectCallback;
    void* _onDisconnectUserData;

    // Flag to prevent double-free
    bool _disposed;

public:
    ProSimConnectWrapper();
    ~ProSimConnectWrapper();

    // Connection methods
    BridgeResult Connect(const char* host, bool synchronous);
    bool IsConnected();
    void SetPriorityMode(bool priority);

    // Callback registration
    void SetOnConnect(ConnectionCallback callback, void* userData);
    void SetOnDisconnect(ConnectionCallback callback, void* userData);

    // Access to managed connection (for DataRef creation)
    ProSimSDK::ProSimConnect^ GetManagedConnection() { return _connection; }

    // Called by the event bridge
    void FireOnConnect();
    void FireOnDisconnect();
};

// ============================================================================
// DataRefWrapper
// Native class that wraps a managed DataRef instance
// ============================================================================

class DataRefWrapper {
private:
    msclr::gcroot<ProSimSDK::DataRef^> _dataRef;
    msclr::gcroot<DataRefEventBridge^> _eventBridge;

    // Native callback storage
    DataRefChangeCallback _onDataChangeCallback;
    void* _onDataChangeUserData;

    // Store the name for C access
    char* _nameBuffer;

    // Flag to prevent double-free
    bool _disposed;

public:
    DataRefWrapper(const char* name, int interval, ProSimConnectWrapper* connection, bool registerNow);
    ~DataRefWrapper();

    // Registration
    BridgeResult Register();

    // Name access
    const char* GetName();

    // Value getters
    BridgeResult GetInt(int32_t* outValue);
    BridgeResult GetDouble(double* outValue);
    BridgeResult GetBool(bool* outValue);
    BridgeResult GetString(char* buffer, int32_t bufferSize);
    BridgeResult GetDateTime(DateTime* outValue);

    // Value setters
    BridgeResult SetInt(int32_t value);
    BridgeResult SetDouble(double value);
    BridgeResult SetBool(bool value);
    BridgeResult SetString(const char* value);
    BridgeResult SetDateTime(const DateTime* value);
    BridgeResult SetReposition(const RepositionData* data);

    // Callback registration
    void SetOnDataChange(DataRefChangeCallback callback, void* userData);

    // Called by the event bridge
    void FireOnDataChange();
};