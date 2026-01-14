#include "ProSimBridge.h"
#include "pch.h"

int main() {
    void* prosim = ProSim_Connect("localhost");

    if (ProSim_IsConnected(prosim)) {
        double altitude = ProSim_ReadDataRef(prosim, "Aircraft.Altitude");
        ProSim_WriteDataRef(prosim, "Aircraft.Heading", 180.0);
    }

    ProSim_Disconnect(prosim);
    ProSim_Destroy(prosim);
}