#include "ProSimBridge.h"
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    // Create a ProSim connection instance
    void* prosim = ProSim_Create();
    if (!prosim) {
        printf("Failed to create ProSimConnect instance\n");
        printf("Error: %s\n", ProSim_GetLastError());
        return 1;
    }
    printf("ProSimConnect instance created successfully\n");

    // Connect to ProSim
    printf("Connecting to localhost...\n");
    BridgeResult result = ProSim_Connect(prosim, "localhost", true);
    if (result != BRIDGE_OK) {
        printf("Failed to connect to ProSim (error code: %d)\n", result);
        printf("Error: %s\n", ProSim_GetLastError());
        ProSim_Destroy(prosim);
        return 1;
    }
    printf("Connected successfully\n");

    // Check connection status
    bool connected = false;
    result = ProSim_IsConnected(prosim, &connected);
    if (result != BRIDGE_OK) {
        printf("Failed to check connection status (error code: %d)\n", result);
        printf("Error: %s\n", ProSim_GetLastError());
    } else {
        printf("Connection status: %s\n", connected ? "Connected" : "Not Connected");
    }

    if (connected) {
        // Read altitude DataRef
        printf("\nReading Aircraft.Altitude...\n");
        double altitude = 0.0;
        result = ProSim_ReadDataRef(prosim, "Aircraft.Altitude", &altitude);
        if (result != BRIDGE_OK) {
            printf("Failed to read altitude (error code: %d)\n", result);
            printf("Error: %s\n", ProSim_GetLastError());
        } else {
            printf("Current altitude: %.2f feet\n", altitude);
        }

        // Write heading DataRef
        printf("\nWriting Aircraft.Heading to 180.0...\n");
        result = ProSim_WriteDataRef(prosim, "Aircraft.Heading", 180.0);
        if (result != BRIDGE_OK) {
            printf("Failed to write heading (error code: %d)\n", result);
            printf("Error: %s\n", ProSim_GetLastError());
        } else {
            printf("Heading set successfully\n");
        }

        // Read heading back to verify
        printf("\nVerifying heading...\n");
        double heading = 0.0;
        result = ProSim_ReadDataRef(prosim, "Aircraft.Heading", &heading);
        if (result != BRIDGE_OK) {
            printf("Failed to read heading (error code: %d)\n", result);
            printf("Error: %s\n", ProSim_GetLastError());
        } else {
            printf("Current heading: %.2f degrees\n", heading);
        }

        // Test reading a non-existent DataRef to demonstrate error handling
        printf("\nTesting error handling with invalid DataRef...\n");
        double invalid = 0.0;
        result = ProSim_ReadDataRef(prosim, "Invalid.DataRef", &invalid);
        if (result != BRIDGE_OK) {
            printf("Expected error occurred (error code: %d)\n", result);
            printf("Error: %s\n", ProSim_GetLastError());
        }

        // ===== NEW DATAREF API EXAMPLES =====
        printf("\n========================================\n");
        printf("Testing New DataRef API (Phase 3)\n");
        printf("========================================\n");

        // Example 1: Integer DataRef
        printf("\n--- Integer DataRef Example ---\n");
        DataRefHandle altitudeRef = DataRef_Create("Aircraft.Altitude", 100, prosim, true);
        if (altitudeRef) {
            printf("Created DataRef: Aircraft.Altitude\n");
            
            // Get current altitude as integer
            int32_t altInt = 0;
            result = DataRef_GetInt(altitudeRef, &altInt);
            if (result == BRIDGE_OK) {
                printf("Altitude (int): %d feet\n", altInt);
            } else {
                printf("Failed to get altitude as int (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            // Set altitude using integer
            printf("Setting altitude to 5000 feet...\n");
            result = DataRef_SetInt(altitudeRef, 5000);
            if (result == BRIDGE_OK) {
                printf("Altitude set successfully\n");
                
                // Read back to verify
                result = DataRef_GetInt(altitudeRef, &altInt);
                if (result == BRIDGE_OK) {
                    printf("Verified altitude: %d feet\n", altInt);
                }
            } else {
                printf("Failed to set altitude (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            DataRef_Destroy(altitudeRef);
            printf("DataRef destroyed\n");
        } else {
            printf("Failed to create altitude DataRef\n");
            printf("Error: %s\n", ProSim_GetLastError());
        }

        // Example 2: Double DataRef
        printf("\n--- Double DataRef Example ---\n");
        DataRefHandle headingRef = DataRef_Create("Aircraft.Heading", 100, prosim, true);
        if (headingRef) {
            printf("Created DataRef: Aircraft.Heading\n");
            
            // Get current heading as double
            double hdg = 0.0;
            result = DataRef_GetDouble(headingRef, &hdg);
            if (result == BRIDGE_OK) {
                printf("Heading (double): %.2f degrees\n", hdg);
            } else {
                printf("Failed to get heading (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            // Set heading using double
            printf("Setting heading to 270.5 degrees...\n");
            result = DataRef_SetDouble(headingRef, 270.5);
            if (result == BRIDGE_OK) {
                printf("Heading set successfully\n");
                
                // Read back to verify
                result = DataRef_GetDouble(headingRef, &hdg);
                if (result == BRIDGE_OK) {
                    printf("Verified heading: %.2f degrees\n", hdg);
                }
            } else {
                printf("Failed to set heading (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            DataRef_Destroy(headingRef);
            printf("DataRef destroyed\n");
        } else {
            printf("Failed to create heading DataRef\n");
            printf("Error: %s\n", ProSim_GetLastError());
        }

        // Example 3: Boolean DataRef
        printf("\n--- Boolean DataRef Example ---\n");
        DataRefHandle gearDownRef = DataRef_Create("Gear.Down", 100, prosim, true);
        if (gearDownRef) {
            printf("Created DataRef: Gear.Down\n");
            
            // Get gear down status
            bool gearDown = false;
            result = DataRef_GetBool(gearDownRef, &gearDown);
            if (result == BRIDGE_OK) {
                printf("Gear Down: %s\n", gearDown ? "true" : "false");
            } else {
                printf("Failed to get gear status (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            // Toggle gear
            printf("Setting gear to down...\n");
            result = DataRef_SetBool(gearDownRef, true);
            if (result == BRIDGE_OK) {
                printf("Gear set successfully\n");
                
                // Read back to verify
                result = DataRef_GetBool(gearDownRef, &gearDown);
                if (result == BRIDGE_OK) {
                    printf("Verified gear down: %s\n", gearDown ? "true" : "false");
                }
            } else {
                printf("Failed to set gear (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            DataRef_Destroy(gearDownRef);
            printf("DataRef destroyed\n");
        } else {
            printf("Failed to create gear DataRef\n");
            printf("Error: %s\n", ProSim_GetLastError());
        }

        // Example 4: String DataRef
        printf("\n--- String DataRef Example ---\n");
        DataRefHandle aircraftTypeRef = DataRef_Create("Aircraft.Type", 100, prosim, true);
        if (aircraftTypeRef) {
            printf("Created DataRef: Aircraft.Type\n");
            
            // Get aircraft type
            char typeBuffer[256];
            result = DataRef_GetString(aircraftTypeRef, typeBuffer, sizeof(typeBuffer));
            if (result == BRIDGE_OK) {
                printf("Aircraft Type: %s\n", typeBuffer);
            } else {
                printf("Failed to get aircraft type (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            // Set aircraft type
            printf("Setting aircraft type to 'B737-800'...\n");
            result = DataRef_SetString(aircraftTypeRef, "B737-800");
            if (result == BRIDGE_OK) {
                printf("Aircraft type set successfully\n");
                
                // Read back to verify
                result = DataRef_GetString(aircraftTypeRef, typeBuffer, sizeof(typeBuffer));
                if (result == BRIDGE_OK) {
                    printf("Verified aircraft type: %s\n", typeBuffer);
                }
            } else {
                printf("Failed to set aircraft type (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            DataRef_Destroy(aircraftTypeRef);
            printf("DataRef destroyed\n");
        } else {
            printf("Failed to create aircraft type DataRef\n");
            printf("Error: %s\n", ProSim_GetLastError());
        }

        // Example 5: DataRef_GetName test
        printf("\n--- DataRef_GetName Example ---\n");
        DataRefHandle speedRef = DataRef_Create("Aircraft.Speed", 100, prosim, true);
        if (speedRef) {
            char nameBuffer[256];
            result = DataRef_GetName(speedRef, nameBuffer, sizeof(nameBuffer));
            if (result == BRIDGE_OK) {
                printf("DataRef name retrieved: %s\n", nameBuffer);
            } else {
                printf("Failed to get DataRef name (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            DataRef_Destroy(speedRef);
        }

        // Example 6: Delayed registration
        printf("\n--- Delayed Registration Example ---\n");
        DataRefHandle delayedRef = DataRef_Create("Aircraft.VerticalSpeed", 100, prosim, false);
        if (delayedRef) {
            printf("Created DataRef without auto-registration\n");
            
            printf("Manually registering DataRef...\n");
            result = DataRef_Register(delayedRef);
            if (result == BRIDGE_OK) {
                printf("DataRef registered successfully\n");
                
                // Now we can read/write
                int32_t vs = 0;
                result = DataRef_GetInt(delayedRef, &vs);
                if (result == BRIDGE_OK) {
                    printf("Vertical Speed: %d ft/min\n", vs);
                }
            } else {
                printf("Failed to register DataRef (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            DataRef_Destroy(delayedRef);
            printf("DataRef destroyed\n");
        }

        printf("\n========================================\n");
        printf("DataRef API Examples Complete\n");
        printf("========================================\n");

        // ===== CALLBACK EXAMPLES =====
        printf("\n========================================\n");
        printf("Testing Callback System (Phase 4)\n");
        printf("========================================\n");

        // Example 1: Connection callbacks
        printf("\n--- Connection Callbacks Example ---\n");
        printf("Note: Connection callbacks fire when connection state changes\n");
        printf("Registering callbacks...\n");

        // Define callback functions
        auto onConnect = [](void* userData) {
            printf("*** CALLBACK: Connected to ProSim! ***\n");
        };

        auto onDisconnect = [](void* userData) {
            printf("*** CALLBACK: Disconnected from ProSim! ***\n");
        };

        // Register callbacks
        result = ProSim_SetOnConnect(prosim, onConnect, nullptr);
        if (result == BRIDGE_OK) {
            printf("Connect callback registered\n");
        } else {
            printf("Failed to register connect callback (error code: %d)\n", result);
            printf("Error: %s\n", ProSim_GetLastError());
        }

        result = ProSim_SetOnDisconnect(prosim, onDisconnect, nullptr);
        if (result == BRIDGE_OK) {
            printf("Disconnect callback registered\n");
        } else {
            printf("Failed to register disconnect callback (error code: %d)\n", result);
            printf("Error: %s\n", ProSim_GetLastError());
        }

        printf("Note: Callbacks will fire on next connect/disconnect event\n");

        // Example 2: DataRef change callback
        printf("\n--- DataRef Change Callback Example ---\n");
        
        // Create a DataRef for monitoring
        DataRefHandle monitoredAlt = DataRef_Create("Aircraft.Altitude", 100, prosim, true);
        if (monitoredAlt) {
            printf("Created DataRef: Aircraft.Altitude for monitoring\n");

            // Define callback for data changes
            auto onAltitudeChange = [](DataRefHandle handle, void* userData) {
                // Read the new value
                int32_t newAlt = 0;
                if (DataRef_GetInt(handle, &newAlt) == BRIDGE_OK) {
                    printf("*** CALLBACK: Altitude changed to %d feet! ***\n", newAlt);
                }
            };

            // Register the callback
            result = DataRef_SetOnDataChange(monitoredAlt, onAltitudeChange, nullptr);
            if (result == BRIDGE_OK) {
                printf("Data change callback registered\n");
                printf("Callback will fire when altitude value changes\n");
            } else {
                printf("Failed to register callback (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            // Trigger a change to demonstrate the callback
            printf("\nTriggering altitude change...\n");
            result = DataRef_SetInt(monitoredAlt, 10000);
            if (result == BRIDGE_OK) {
                printf("Altitude set to 10000 feet\n");
                printf("(Callback should fire above)\n");
            }

            // Wait briefly to allow callback to fire
            printf("\nWaiting for callbacks to process...\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            DataRef_Destroy(monitoredAlt);
            printf("Monitored DataRef destroyed\n");
        }

        // Example 3: Callback with user data
        printf("\n--- Callback with User Data Example ---\n");
        
        struct CallbackContext {
            int callCount;
            const char* name;
        };

        CallbackContext ctx = { 0, "SpeedMonitor" };

        DataRefHandle speedMonitor = DataRef_Create("Aircraft.Speed", 100, prosim, true);
        if (speedMonitor) {
            printf("Created DataRef: Aircraft.Speed with context\n");

            // Callback that uses user data
            auto onSpeedChange = [](DataRefHandle handle, void* userData) {
                CallbackContext* context = static_cast<CallbackContext*>(userData);
                context->callCount++;
                
                double speed = 0.0;
                if (DataRef_GetDouble(handle, &speed) == BRIDGE_OK) {
                    printf("*** CALLBACK [%s]: Speed changed to %.2f knots (call #%d) ***\n", 
                           context->name, speed, context->callCount);
                }
            };

            result = DataRef_SetOnDataChange(speedMonitor, onSpeedChange, &ctx);
            if (result == BRIDGE_OK) {
                printf("Callback registered with user data\n");
                
                // Trigger multiple changes
                printf("\nTriggering speed changes...\n");
                DataRef_SetDouble(speedMonitor, 150.5);
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                
                DataRef_SetDouble(speedMonitor, 200.0);
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                
                printf("\nTotal callbacks fired: %d\n", ctx.callCount);
            }

            DataRef_Destroy(speedMonitor);
            printf("Speed monitor destroyed\n");
        }

        printf("\n========================================\n");
        printf("Callback System Examples Complete\n");
        printf("========================================\n");

        // ===== ADVANCED FEATURES EXAMPLES (PHASE 5) =====
        printf("\n========================================\n");
        printf("Testing Advanced Features (Phase 5)\n");
        printf("========================================\n");

        // Example 1: Priority Mode
        printf("\n--- Priority Mode Example ---\n");
        printf("Setting SDK Priority Mode to true...\n");
        result = ProSim_SetPriorityMode(prosim, true);
        if (result == BRIDGE_OK) {
            printf("Priority mode enabled successfully\n");
            printf("SDK commands now have priority over UI commands\n");
        } else {
            printf("Failed to set priority mode (error code: %d)\n", result);
            printf("Error: %s\n", ProSim_GetLastError());
        }

        // Example 2: DateTime DataRef
        printf("\n--- DateTime DataRef Example ---\n");
        DataRefHandle simTimeRef = DataRef_Create("System.Time", 100, prosim, true);
        if (simTimeRef) {
            printf("Created DataRef: System.Time\n");
            
            // Get current simulator time
            DateTime currentTime = {0};
            result = DataRef_GetDateTime(simTimeRef, &currentTime);
            if (result == BRIDGE_OK) {
                printf("Current Sim Time: %04d-%02d-%02d %02d:%02d:%02d.%03d\n",
                       currentTime.year, currentTime.month, currentTime.day,
                       currentTime.hour, currentTime.minute, currentTime.second,
                       currentTime.millisecond);
            } else {
                printf("Failed to get DateTime (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            // Set a new time
            printf("\nSetting simulator time to 2025-12-25 14:30:00...\n");
            DateTime newTime = {0};
            newTime.year = 2025;
            newTime.month = 12;
            newTime.day = 25;
            newTime.hour = 14;
            newTime.minute = 30;
            newTime.second = 0;
            newTime.millisecond = 0;

            result = DataRef_SetDateTime(simTimeRef, &newTime);
            if (result == BRIDGE_OK) {
                printf("Simulator time set successfully\n");
                
                // Read back to verify
                DateTime verifyTime = {0};
                result = DataRef_GetDateTime(simTimeRef, &verifyTime);
                if (result == BRIDGE_OK) {
                    printf("Verified time: %04d-%02d-%02d %02d:%02d:%02d\n",
                           verifyTime.year, verifyTime.month, verifyTime.day,
                           verifyTime.hour, verifyTime.minute, verifyTime.second);
                }
            } else {
                printf("Failed to set DateTime (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            DataRef_Destroy(simTimeRef);
            printf("DateTime DataRef destroyed\n");
        } else {
            printf("Failed to create time DataRef\n");
            printf("Error: %s\n", ProSim_GetLastError());
        }

        // Example 3: RepositionData
        printf("\n--- Aircraft Reposition Example ---\n");
        DataRefHandle repositionRef = DataRef_Create("Aircraft.Position", 100, prosim, true);
        if (repositionRef) {
            printf("Created DataRef: Aircraft.Position\n");
            
            // Set up reposition data
            RepositionData reposition = {0};
            reposition.latitude = 47.6062;        // Seattle latitude
            reposition.longitude = -122.3321;     // Seattle longitude
            reposition.altitude = 500.0;          // 500 feet
            reposition.heading_magnetic = 360.0;  // North
            reposition.pitch = 0.0;
            reposition.bank = 0.0;
            reposition.ias = 150.0;               // 150 knots
            reposition.on_ground = false;

            printf("\nRepositioning aircraft to Seattle (47.6062°N, 122.3321°W)...\n");
            printf("  Altitude: %.0f ft\n", reposition.altitude);
            printf("  Heading: %.0f° (magnetic)\n", reposition.heading_magnetic);
            printf("  IAS: %.0f knots\n", reposition.ias);
            printf("  On Ground: %s\n", reposition.on_ground ? "Yes" : "No");

            result = DataRef_SetReposition(repositionRef, &reposition);
            if (result == BRIDGE_OK) {
                printf("\nAircraft repositioned successfully!\n");
                printf("The aircraft is now at the new location\n");
            } else {
                printf("Failed to reposition aircraft (error code: %d)\n", result);
                printf("Error: %s\n", ProSim_GetLastError());
            }

            DataRef_Destroy(repositionRef);
            printf("Reposition DataRef destroyed\n");
        } else {
            printf("Failed to create reposition DataRef\n");
            printf("Error: %s\n", ProSim_GetLastError());
        }

        // Example 4: Combined Advanced Features
        printf("\n--- Combined Advanced Features Example ---\n");
        printf("Demonstrating priority mode with aircraft state changes...\n");
        
        // Enable priority mode for critical updates
        ProSim_SetPriorityMode(prosim, true);
        printf("Priority mode enabled for critical updates\n");

        // Create multiple DataRefs for coordinated updates
        DataRefHandle altRef = DataRef_Create("Aircraft.Altitude", 50, prosim, true);
        DataRefHandle spdRef = DataRef_Create("Aircraft.Speed", 50, prosim, true);
        
        if (altRef && spdRef) {
            printf("\nPerforming coordinated altitude and speed changes...\n");
            
            // Set cruise altitude and speed
            DataRef_SetInt(altRef, 35000);
            DataRef_SetDouble(spdRef, 450.0);
            
            printf("Set cruise parameters: 35,000 ft at 450 knots\n");
            printf("Priority mode ensures these updates take precedence\n");

            DataRef_Destroy(altRef);
            DataRef_Destroy(spdRef);
        }

        // Disable priority mode when done
        ProSim_SetPriorityMode(prosim, false);
        printf("Priority mode disabled\n");

        printf("\n========================================\n");
        printf("Advanced Features Examples Complete\n");
        printf("========================================\n");
    }

    // Disconnect and cleanup
    printf("\nDisconnecting...\n");
    ProSim_Disconnect(prosim);
    ProSim_Destroy(prosim);
    printf("Cleanup complete\n");

    return 0;
}