// ManagedWrapper.cpp
// Implementation of native wrapper classes for managed ProSimSDK types

#include "pch.h"
#include "ManagedWrapper.h"
#include <cstring>

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace ProSimSDK;

// Helper function to store exception message for error reporting
static void StoreExceptionMessage(Exception^ ex) {
    if (ex != nullptr) {
        try {
            // Build detailed error message with exception type and message
            String^ fullMessage = ex->GetType()->FullName + ": " + ex->Message;

            // Include inner exception chain if present
            Exception^ inner = ex->InnerException;
            while (inner != nullptr) {
                fullMessage += "\n  --> " + inner->GetType()->FullName + ": " + inner->Message;
                inner = inner->InnerException;
            }

            // Include stack trace if available
            if (ex->StackTrace != nullptr && ex->StackTrace->Length > 0) {
                fullMessage += "\nStack trace:\n" + ex->StackTrace;
            }

            IntPtr ptr = Marshal::StringToHGlobalAnsi(fullMessage);
            try {
                ProSim_SetLastError(static_cast<const char*>(ptr.ToPointer()));
            }
            finally {
                Marshal::FreeHGlobal(ptr);
            }
        }
        catch (...) {
            ProSim_SetLastError("Exception occurred (failed to get details)");
        }
    }
}

// ============================================================================
// ConnectionEventBridge Implementation
// ============================================================================

void ConnectionEventBridge::OnConnect() {
    if (_nativeWrapper) {
        _nativeWrapper->FireOnConnect();
    }
}

void ConnectionEventBridge::OnDisconnect() {
    if (_nativeWrapper) {
        _nativeWrapper->FireOnDisconnect();
    }
}

// ============================================================================
// ProSimConnectWrapper Implementation
// ============================================================================

ProSimConnectWrapper::ProSimConnectWrapper()
    : _disposed(false)
    , _onConnectCallback(nullptr)
    , _onConnectUserData(nullptr)
    , _onDisconnectCallback(nullptr)
    , _onDisconnectUserData(nullptr)
{
    _connection = gcnew ProSimConnect();
    _eventBridge = gcnew ConnectionEventBridge(this);

    // Subscribe to managed events using the bridge class
    _connection->onConnect += gcnew ProSimConnect::connectionChangedDelegate(_eventBridge, &ConnectionEventBridge::OnConnect);
    _connection->onDisconnect += gcnew ProSimConnect::connectionChangedDelegate(_eventBridge, &ConnectionEventBridge::OnDisconnect);
}

ProSimConnectWrapper::~ProSimConnectWrapper() {
    if (!_disposed) {
        _disposed = true;
        try {
            // Unsubscribe from events before disposing
            ProSimConnect^ conn = _connection;
            ConnectionEventBridge^ bridge = _eventBridge;
            if (conn != nullptr && bridge != nullptr) {
                conn->onConnect -= gcnew ProSimConnect::connectionChangedDelegate(bridge, &ConnectionEventBridge::OnConnect);
                conn->onDisconnect -= gcnew ProSimConnect::connectionChangedDelegate(bridge, &ConnectionEventBridge::OnDisconnect);
            }

            // Dispose the connection (delete invokes IDisposable::Dispose in C++/CLI)
            if (conn != nullptr) {
                try {
                    delete conn;
                }
                catch (...) {
                    // Ignore disposal errors
                }
            }
        }
        catch (...) {
            // Ignore exceptions during cleanup
        }
    }
}

BridgeResult ProSimConnectWrapper::Connect(const char* host, bool synchronous) {
    try {
        String^ managedHost = gcnew String(host);
        _connection->Connect(managedHost, synchronous);
        return BRIDGE_OK;
    }
    catch (NotConnectedException^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_CONNECTION_FAILED;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

bool ProSimConnectWrapper::IsConnected() {
    try {
        return _connection->isConnected;
    }
    catch (...) {
        return false;
    }
}

void ProSimConnectWrapper::SetPriorityMode(bool priority) {
    try {
        _connection->setSDKPriorityMode(priority);
    }
    catch (...) {
        // Ignore errors
    }
}

void ProSimConnectWrapper::SetOnConnect(ConnectionCallback callback, void* userData) {
    _onConnectCallback = callback;
    _onConnectUserData = userData;
}

void ProSimConnectWrapper::SetOnDisconnect(ConnectionCallback callback, void* userData) {
    _onDisconnectCallback = callback;
    _onDisconnectUserData = userData;
}

void ProSimConnectWrapper::FireOnConnect() {
    if (_onConnectCallback) {
        _onConnectCallback(_onConnectUserData);
    }
}

void ProSimConnectWrapper::FireOnDisconnect() {
    if (_onDisconnectCallback) {
        _onDisconnectCallback(_onDisconnectUserData);
    }
}

// ============================================================================
// DataRefEventBridge Implementation
// ============================================================================

void DataRefEventBridge::OnDataChange(DataRef^ dataRef) {
    if (_nativeWrapper) {
        _nativeWrapper->FireOnDataChange();
    }
}

// ============================================================================
// DataRefWrapper Implementation
// ============================================================================

DataRefWrapper::DataRefWrapper(const char* name, int interval, ProSimConnectWrapper* connection, bool registerNow)
    : _nameBuffer(nullptr)
    , _disposed(false)
    , _onDataChangeCallback(nullptr)
    , _onDataChangeUserData(nullptr)
{
    String^ managedName = gcnew String(name);
    ProSimConnect^ conn = connection->GetManagedConnection();

    _dataRef = gcnew DataRef(managedName, interval, conn, registerNow);
    _eventBridge = gcnew DataRefEventBridge(this);

    // Store name for later retrieval
    size_t len = strlen(name) + 1;
    _nameBuffer = new char[len];
    strcpy_s(_nameBuffer, len, name);

    // Subscribe to data change events using the bridge class
    _dataRef->onDataChange += gcnew DataRef::onDataChangeDelegate(_eventBridge, &DataRefEventBridge::OnDataChange);
}

DataRefWrapper::~DataRefWrapper() {
    if (!_disposed) {
        _disposed = true;
        try {
            // Unsubscribe from events before disposing
            DataRef^ dr = _dataRef;
            DataRefEventBridge^ bridge = _eventBridge;
            if (dr != nullptr && bridge != nullptr) {
                dr->onDataChange -= gcnew DataRef::onDataChangeDelegate(bridge, &DataRefEventBridge::OnDataChange);
            }

            // Dispose the DataRef (delete invokes IDisposable::Dispose in C++/CLI)
            if (dr != nullptr) {
                try {
                    delete dr;
                }
                catch (...) {
                    // Ignore disposal errors
                }
            }
        }
        catch (...) {
            // Ignore exceptions during cleanup
        }

        // Free name buffer
        if (_nameBuffer) {
            delete[] _nameBuffer;
            _nameBuffer = nullptr;
        }
    }
}

BridgeResult DataRefWrapper::Register() {
    try {
        // Note: DataRef constructor with registerNow parameter handles registration
        // This method is for explicit registration if needed
        return BRIDGE_OK;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

const char* DataRefWrapper::GetName() {
    return _nameBuffer;
}

BridgeResult DataRefWrapper::GetInt(int32_t* outValue) {
    if (!outValue) return BRIDGE_ERR_INVALID_ARGUMENT;

    try {
        Object^ val = _dataRef->value;
        *outValue = Convert::ToInt32(val);
        return BRIDGE_OK;
    }
    catch (DataRefNotReady^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_DATAREF_NOT_READY;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

BridgeResult DataRefWrapper::GetDouble(double* outValue) {
    if (!outValue) return BRIDGE_ERR_INVALID_ARGUMENT;

    try {
        Object^ val = _dataRef->value;
        *outValue = Convert::ToDouble(val);
        return BRIDGE_OK;
    }
    catch (DataRefNotReady^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_DATAREF_NOT_READY;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

BridgeResult DataRefWrapper::GetBool(bool* outValue) {
    if (!outValue) return BRIDGE_ERR_INVALID_ARGUMENT;

    try {
        Object^ val = _dataRef->value;
        *outValue = Convert::ToBoolean(val);
        return BRIDGE_OK;
    }
    catch (DataRefNotReady^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_DATAREF_NOT_READY;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

BridgeResult DataRefWrapper::GetString(char* buffer, int32_t bufferSize) {
    if (!buffer || bufferSize <= 0) return BRIDGE_ERR_INVALID_ARGUMENT;

    try {
        Object^ val = _dataRef->value;
        String^ str = val->ToString();

        // Check if buffer is large enough
        int requiredSize = str->Length + 1;
        if (requiredSize > bufferSize) {
            return (BridgeResult)requiredSize; // Return required size
        }

        // Convert managed string to native string
        IntPtr ptr = Marshal::StringToHGlobalAnsi(str);
        try {
            strncpy_s(buffer, bufferSize, static_cast<const char*>(ptr.ToPointer()), _TRUNCATE);
        }
        finally {
            Marshal::FreeHGlobal(ptr);
        }

        return BRIDGE_OK;
    }
    catch (DataRefNotReady^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_DATAREF_NOT_READY;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

BridgeResult DataRefWrapper::SetInt(int32_t value) {
    try {
        _dataRef->value = value;
        return BRIDGE_OK;
    }
    catch (InvalidData^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_INVALID_DATA;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

BridgeResult DataRefWrapper::SetDouble(double value) {
    try {
        _dataRef->value = value;
        return BRIDGE_OK;
    }
    catch (InvalidData^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_INVALID_DATA;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

BridgeResult DataRefWrapper::SetBool(bool value) {
    try {
        _dataRef->value = value;
        return BRIDGE_OK;
    }
    catch (InvalidData^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_INVALID_DATA;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

BridgeResult DataRefWrapper::SetString(const char* value) {
    if (!value) return BRIDGE_ERR_INVALID_ARGUMENT;

    try {
        String^ managedValue = gcnew String(value);
        _dataRef->value = managedValue;
        return BRIDGE_OK;
    }
    catch (InvalidData^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_INVALID_DATA;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

BridgeResult DataRefWrapper::GetDateTime(::DateTime* outValue) {
    if (!outValue) return BRIDGE_ERR_INVALID_ARGUMENT;

    try {
        Object^ val = _dataRef->value;
        System::DateTime dt = Convert::ToDateTime(val);
        outValue->year = dt.Year;
        outValue->month = dt.Month;
        outValue->day = dt.Day;
        outValue->hour = dt.Hour;
        outValue->minute = dt.Minute;
        outValue->second = dt.Second;
        outValue->millisecond = dt.Millisecond;
        return BRIDGE_OK;
    }
    catch (DataRefNotReady^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_DATAREF_NOT_READY;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

BridgeResult DataRefWrapper::SetDateTime(const ::DateTime* value) {
    if (!value) return BRIDGE_ERR_INVALID_ARGUMENT;

    try {
        System::DateTime^ dt = gcnew System::DateTime(
            value->year,
            value->month,
            value->day,
            value->hour,
            value->minute,
            value->second,
            value->millisecond
        );
        _dataRef->value = dt;
        return BRIDGE_OK;
    }
    catch (InvalidData^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_INVALID_DATA;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

BridgeResult DataRefWrapper::SetReposition(const ::RepositionData* data) {
    if (!data) return BRIDGE_ERR_INVALID_ARGUMENT;

    try {
        ProSimSDK::RepositionData^ reposition = gcnew ProSimSDK::RepositionData();
        reposition->Latitude = data->latitude;
        reposition->Longitude = data->longitude;
        reposition->Altitude = data->altitude;
        reposition->HeadingMagnetic = data->heading_magnetic;
        reposition->Pitch = data->pitch;
        reposition->Bank = data->bank;
        reposition->Ias = data->ias;
        reposition->OnGround = data->on_ground;

        _dataRef->value = reposition;
        return BRIDGE_OK;
    }
    catch (InvalidData^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_INVALID_DATA;
    }
    catch (Exception^ ex) {
        StoreExceptionMessage(ex);
        return BRIDGE_ERR_EXCEPTION;
    }
}

void DataRefWrapper::SetOnDataChange(DataRefChangeCallback callback, void* userData) {
    _onDataChangeCallback = callback;
    _onDataChangeUserData = userData;
}

void DataRefWrapper::FireOnDataChange() {
    if (_onDataChangeCallback) {
        _onDataChangeCallback(static_cast<DataRefHandle>(this), _onDataChangeUserData);
    }
}