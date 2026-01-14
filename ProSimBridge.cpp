#include "pch.h"
#include "ProSimBridge.h"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace ProSimSDK; // Adjust namespace

ref class ProSimWrapper {
public:
    ProSimConnect^ connection;

    ProSimWrapper(String^ host) {
        connection = gcnew ProSimConnect();
        if (host != nullptr)
            connection->Connect(host, true);
        else
            connection->Connect();
    }
};


extern "C" {
    void* ProSim_Connect(const char* host) {
        String^ hostStr = host ? gcnew String(host) : nullptr;
        auto wrapper = gcnew ProSimWrapper(hostStr);
        return GCHandle::ToIntPtr(GCHandle::Alloc(wrapper)).ToPointer();
    }

    void ProSim_Disconnect(void* instance) {
        auto handle = GCHandle::FromIntPtr(IntPtr(instance));
        auto wrapper = (ProSimWrapper^)handle.Target;
        //wrapper->connection->Dispose();
    }

    bool ProSim_IsConnected(void* instance) {
        auto handle = GCHandle::FromIntPtr(IntPtr(instance));
        auto wrapper = (ProSimWrapper^)handle.Target;
        return wrapper->connection->isConnected;
    }

    double ProSim_ReadDataRef(void* instance, const char* name) {
        auto handle = GCHandle::FromIntPtr(IntPtr(instance));
        auto wrapper = (ProSimWrapper^)handle.Target;
        Object^ value = wrapper->connection->ReadDataRef(gcnew String(name));
        return Convert::ToDouble(value);
    }

    void ProSim_WriteDataRef(void* instance, const char* name, double value) {
        auto handle = GCHandle::FromIntPtr(IntPtr(instance));
        auto wrapper = (ProSimWrapper^)handle.Target;
        auto dataRef = gcnew DataRef(gcnew String(name), wrapper->connection);
        dataRef->value = value;
    }

    void ProSim_Destroy(void* instance) {
        auto handle = GCHandle::FromIntPtr(IntPtr(instance));
        handle.Free();
    }
}