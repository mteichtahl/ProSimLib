# ProSimBridge

A high-performance C++/CLI bridge library providing native C API access to ProSim737's .NET SDK. Enables seamless integration of ProSim737 flight simulator with applications written in C, C++, Rust, and other languages that support C FFI.

## Features

### Core Capabilities
- ✅ **Connection Management** - Connect/disconnect with connection state monitoring
- ✅ **DataRef Operations** - Type-safe read/write operations for simulator data
- ✅ **Event System** - Callback-based notifications for state changes
- ✅ **Error Handling** - Comprehensive error codes with detailed messages
- ✅ **Memory Safety** - RAII-based resource management with gcroot<T>

### DataRef Support
- **Type-Specific Operations**
  - Integer (int32_t)
  - Double precision floating point
  - Boolean
  - String
  - DateTime
  - RepositionData (aircraft positioning)

### Advanced Features
- **Lifecycle Management** - Create, register, and destroy DataRefs
- **Callback System** - Connection events and DataRef value change notifications
- **Priority Mode** - SDK command prioritization over UI commands
- **Batch Operations** - Efficient coordinated updates

## Architecture

```
┌─────────────────────────────────────────┐
│         Your Application (C/C++)        │
│         Or Rust/Python/etc via FFI      │
└────────────────┬────────────────────────┘
                 │ C API
┌────────────────▼────────────────────────┐
│         ProSimBridge.dll                │
│    ┌──────────────────────────────┐    │
│    │    C API Layer               │    │
│    │  (ProSimBridge.cpp/.h)       │    │
│    └──────────┬───────────────────┘    │
│               │                         │
│    ┌──────────▼───────────────────┐    │
│    │  Native Wrapper Layer        │    │
│    │  (ManagedWrapper.cpp/.h)     │    │
│    │  • ProSimConnectWrapper      │    │
│    │  • DataRefWrapper            │    │
│    │  • Event Bridges             │    │
│    └──────────┬───────────────────┘    │
│               │ gcroot<T>               │
└───────────────┼─────────────────────────┘
                │
┌───────────────▼─────────────────────────┐
│         ProSimSDK.dll (.NET)            │
│    ProSim737 Official SDK               │
└─────────────────────────────────────────┘
```

## Quick Start

### Prerequisites
- Visual Studio 2019 or later with C++/CLI support
- .NET Framework 4.7.2 or later
- ProSim737 installed with ProSimSDK.dll

### Basic Usage

```cpp
#include "ProSimBridge.h"

// Create connection
void* prosim = ProSim_Create();

// Connect to ProSim
BridgeResult result = ProSim_Connect(prosim, "localhost", true);
if (result != BRIDGE_OK) {
    printf("Connection failed: %s\n", ProSim_GetLastError());
    return 1;
}

// Create a DataRef
DataRefHandle altitude = DataRef_Create("Aircraft.Altitude", 100, prosim, true);

// Read value
int32_t alt = 0;
DataRef_GetInt(altitude, &alt);
printf("Altitude: %d feet\n", alt);

// Write value
DataRef_SetInt(altitude, 5000);

// Cleanup
DataRef_Destroy(altitude);
ProSim_Destroy(prosim);
```

## API Reference

### Connection Management

#### `ProSim_Create`
Creates a new ProSimConnect instance.
```cpp
void* ProSim_Create(void);
```
**Returns:** Handle to instance, or NULL on failure

#### `ProSim_Connect`
Connects to ProSim at the specified host.
```cpp
BridgeResult ProSim_Connect(void* instance, const char* host, bool synchronous);
```
**Parameters:**
- `instance` - Handle from ProSim_Create
- `host` - Hostname or IP address ("localhost" for local)
- `synchronous` - If true, blocks until connected

**Returns:** BRIDGE_OK on success, error code on failure

#### `ProSim_IsConnected`
Checks connection status.
```cpp
BridgeResult ProSim_IsConnected(void* instance, bool* out_connected);
```

#### `ProSim_Destroy`
Destroys instance and releases resources.
```cpp
void ProSim_Destroy(void* instance);
```

### DataRef Lifecycle

#### `DataRef_Create`
Creates a new DataRef instance.
```cpp
DataRefHandle DataRef_Create(const char* name, int32_t interval, 
                              void* connection, bool register_now);
```
**Parameters:**
- `name` - DataRef name (e.g., "Aircraft.Altitude")
- `interval` - Polling interval in milliseconds
- `connection` - ProSimConnect handle
- `register_now` - If true, registers immediately

**Returns:** DataRef handle, or NULL on failure

#### `DataRef_Destroy`
Destroys DataRef and releases resources.
```cpp
void DataRef_Destroy(DataRefHandle handle);
```

### Type-Specific Operations

#### Integer Operations
```cpp
BridgeResult DataRef_GetInt(DataRefHandle handle, int32_t* out_value);
BridgeResult DataRef_SetInt(DataRefHandle handle, int32_t value);
```

#### Double Operations
```cpp
BridgeResult DataRef_GetDouble(DataRefHandle handle, double* out_value);
BridgeResult DataRef_SetDouble(DataRefHandle handle, double value);
```

#### Boolean Operations
```cpp
BridgeResult DataRef_GetBool(DataRefHandle handle, bool* out_value);
BridgeResult DataRef_SetBool(DataRefHandle handle, bool value);
```

#### String Operations
```cpp
BridgeResult DataRef_GetString(DataRefHandle handle, char* out_buffer, 
                                int32_t buffer_size);
BridgeResult DataRef_SetString(DataRefHandle handle, const char* value);
```

#### DateTime Operations
```cpp
typedef struct {
    int32_t year, month, day;
    int32_t hour, minute, second, millisecond;
} DateTime;

BridgeResult DataRef_GetDateTime(DataRefHandle handle, DateTime* out_value);
BridgeResult DataRef_SetDateTime(DataRefHandle handle, const DateTime* value);
```

#### Aircraft Repositioning
```cpp
typedef struct {
    double latitude, longitude, altitude;
    double heading_magnetic, pitch, bank, ias;
    bool on_ground;
} RepositionData;

BridgeResult DataRef_SetReposition(DataRefHandle handle, 
                                    const RepositionData* data);
```

### Callback System

#### Connection Callbacks
```cpp
typedef void (*ConnectionCallback)(void* user_data);

BridgeResult ProSim_SetOnConnect(void* instance, ConnectionCallback callback, 
                                  void* user_data);
BridgeResult ProSim_SetOnDisconnect(void* instance, ConnectionCallback callback,
                                     void* user_data);
```

#### DataRef Change Callbacks
```cpp
typedef void (*DataRefChangeCallback)(DataRefHandle handle, void* user_data);

BridgeResult DataRef_SetOnDataChange(DataRefHandle handle, 
                                      DataRefChangeCallback callback,
                                      void* user_data);
```

**Example:**
```cpp
void OnAltitudeChange(DataRefHandle handle, void* userData) {
    int32_t altitude = 0;
    DataRef_GetInt(handle, &altitude);
    printf("Altitude changed: %d feet\n", altitude);
}

DataRef_SetOnDataChange(altitudeRef, OnAltitudeChange, nullptr);
```

### Advanced Features

#### Priority Mode
Enables SDK commands to take priority over UI commands.
```cpp
BridgeResult ProSim_SetPriorityMode(void* instance, bool priority);
```

### Error Handling

#### Error Codes
```cpp
#define BRIDGE_OK                    0
#define BRIDGE_ERR_NULL_HANDLE      -1
#define BRIDGE_ERR_NOT_CONNECTED    -2
#define BRIDGE_ERR_CONNECTION_FAILED -3
#define BRIDGE_ERR_INVALID_ARGUMENT -4
#define BRIDGE_ERR_DATAREF_NOT_FOUND -5
#define BRIDGE_ERR_DATAREF_NOT_READY -6
#define BRIDGE_ERR_INVALID_DATA     -7
#define BRIDGE_ERR_EXCEPTION        -99
```

#### Get Last Error
```cpp
const char* ProSim_GetLastError(void);
```

**Example:**
```cpp
BridgeResult result = ProSim_Connect(prosim, "localhost", true);
if (result != BRIDGE_OK) {
    printf("Error: %s\n", ProSim_GetLastError());
}
```

## Complete Examples

### Example 1: Basic Connection and DataRef Operations
```cpp
#include "ProSimBridge.h"
#include <stdio.h>

int main() {
    // Create and connect
    void* prosim = ProSim_Create();
    if (ProSim_Connect(prosim, "localhost", true) != BRIDGE_OK) {
        printf("Connection failed: %s\n", ProSim_GetLastError());
        return 1;
    }

    // Create DataRefs
    DataRefHandle altitude = DataRef_Create("Aircraft.Altitude", 100, prosim, true);
    DataRefHandle speed = DataRef_Create("Aircraft.Speed", 100, prosim, true);

    // Read values
    int32_t alt;
    double spd;
    DataRef_GetInt(altitude, &alt);
    DataRef_GetDouble(speed, &spd);
    printf("Altitude: %d ft, Speed: %.1f kts\n", alt, spd);

    // Write values
    DataRef_SetInt(altitude, 10000);
    DataRef_SetDouble(speed, 250.0);

    // Cleanup
    DataRef_Destroy(altitude);
    DataRef_Destroy(speed);
    ProSim_Destroy(prosim);
    return 0;
}
```

### Example 2: Event-Driven Programming with Callbacks
```cpp
#include "ProSimBridge.h"
#include <stdio.h>

void OnConnect(void* userData) {
    printf("Connected to ProSim!\n");
}

void OnAltitudeChange(DataRefHandle handle, void* userData) {
    int32_t alt;
    DataRef_GetInt(handle, &alt);
    printf("Altitude changed: %d feet\n", alt);
}

int main() {
    void* prosim = ProSim_Create();
    
    // Register connection callback
    ProSim_SetOnConnect(prosim, OnConnect, nullptr);
    ProSim_Connect(prosim, "localhost", false);  // Async connect

    // Monitor altitude changes
    DataRefHandle altitude = DataRef_Create("Aircraft.Altitude", 100, prosim, true);
    DataRef_SetOnDataChange(altitude, OnAltitudeChange, nullptr);

    // Keep running to receive callbacks
    printf("Monitoring altitude... Press Ctrl+C to exit\n");
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    DataRef_Destroy(altitude);
    ProSim_Destroy(prosim);
    return 0;
}
```

### Example 3: Aircraft Repositioning
```cpp
#include "ProSimBridge.h"
#include <stdio.h>

int main() {
    void* prosim = ProSim_Create();
    ProSim_Connect(prosim, "localhost", true);

    // Enable priority mode for critical updates
    ProSim_SetPriorityMode(prosim, true);

    // Create reposition DataRef
    DataRefHandle position = DataRef_Create("Aircraft.Position", 100, prosim, true);

    // Set up new position (Seattle)
    RepositionData reposition = {
        .latitude = 47.6062,
        .longitude = -122.3321,
        .altitude = 1000.0,
        .heading_magnetic = 360.0,
        .pitch = 0.0,
        .bank = 0.0,
        .ias = 150.0,
        .on_ground = false
    };

    // Reposition aircraft
    if (DataRef_SetReposition(position, &reposition) == BRIDGE_OK) {
        printf("Aircraft repositioned successfully!\n");
    }

    ProSim_SetPriorityMode(prosim, false);
    DataRef_Destroy(position);
    ProSim_Destroy(prosim);
    return 0;
}
```

## Building

### Quick Build (Automated Scripts)

#### Windows
```bash
# Simple build (Release mode)
build.bat

# Debug build
build.bat --debug

# Clean build
build.bat --clean --release

# Build without tests
build.bat --no-tests

# Use Visual Studio 2019
build.bat --vs2019
```

#### Unix/Linux/macOS (Reference - Requires Windows)
```bash
# Make script executable
chmod +x build.sh

# Build (requires Windows environment)
./build.sh --release
```

**Note:** ProSimBridge uses C++/CLI which requires Windows and .NET Framework. The Unix build script is provided for reference and potential Wine-based builds.

### CMake Build (Advanced)

#### Prerequisites
- CMake 3.15 or later
- Visual Studio 2019 or 2022 with C++/CLI support
- .NET Framework 4.7.2 or later
- ProSimSDK.dll in `libs/` directory

#### Configure and Build
```bash
# Create build directory
mkdir build
cd build

# Configure
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release --parallel

# Install (optional)
cmake --install . --config Release --prefix ../install

# Create package (optional)
cpack -C Release
```

#### CMake Options
- `BUILD_TESTS` - Build test executable (default: ON)
- `CMAKE_INSTALL_PREFIX` - Installation directory
- `CMAKE_BUILD_TYPE` - Build configuration (Debug/Release)

#### Build Outputs
```
build/
├── bin/
│   └── Release/
│       ├── ProSimBridge.dll
│       ├── ProSimSDK.dll
│       └── ProSimBridgeTest.exe
└── lib/
    └── Release/
        └── ProSimBridge.lib
```

### Visual Studio (Traditional)

1. Open `ProSimBridge.vcxproj` in Visual Studio
2. Ensure ProSimSDK.dll is in the `libs/` directory
3. Build → Build Solution (Ctrl+Shift+B)
4. Output: `x64/Release/ProSimBridge.dll`

### Build Configurations
- **Debug**: Includes debugging symbols, no optimizations
- **Release**: Optimized for performance

### Dependencies
- ProSimSDK.dll (place in `libs/` directory)
- .NET Framework 4.7.2 or later

## Testing

Run the included test program:
```bash
test.exe
```

The test program demonstrates:
- Connection management
- Legacy DataRef operations
- Type-specific DataRef operations (Phase 3)
- Callback system (Phase 4)
- Advanced features: DateTime, RepositionData, Priority Mode (Phase 5)

## Implementation Phases

This library was developed in 7 phases for parity with the vendor's reference implementation:

1. ✅ **Phase 1**: Error handling foundation
2. ✅ **Phase 2**: Architecture refactoring (GCHandle → gcroot<T>)
3. ✅ **Phase 3**: DataRef enhancement (type-specific operations)
4. ✅ **Phase 4**: Callback system
5. ✅ **Phase 5**: Advanced features (DateTime, RepositionData, Priority Mode)
6. ✅ **Phase 6**: Build configuration (CMake, automated scripts, packaging)
7. ✅ **Phase 7**: Testing & documentation

## Project Structure

```
ProSimBridge/
├── ProSimBridge.h          # C API header
├── ProSimBridge.cpp        # C API implementation
├── ManagedWrapper.h        # Native wrapper classes
├── ManagedWrapper.cpp      # Wrapper implementation
├── pch.h/pch.cpp          # Precompiled headers
├── test.cpp               # Comprehensive test suite
├── libs/
│   └── ProSimSDK.dll      # ProSim .NET SDK
└── README.md              # This file
```

## Best Practices

### Memory Management
- Always call `ProSim_Destroy()` to release connection resources
- Always call `DataRef_Destroy()` to release DataRef resources
- The library uses RAII internally for automatic cleanup

### Error Handling
- Check return codes for all API calls
- Use `ProSim_GetLastError()` for detailed error messages
- Error messages include exception type, message, and stack trace

### Performance
- Use appropriate polling intervals for DataRefs (100ms recommended)
- Enable Priority Mode only when needed for critical updates
- Batch related updates together when possible

### Thread Safety
- Connection and DataRef instances are not thread-safe
- Use one instance per thread or implement external synchronization
- Callbacks are invoked on the event thread

## Troubleshooting

### Connection Issues
```
Error: Connection failed
```
**Solution:** Ensure ProSim737 is running and accepting connections

### DataRef Not Found
```
Error: DataRef not found
```
**Solution:** Verify DataRef name spelling and availability in ProSim

### Type Mismatch
```
Error: Invalid data type
```
**Solution:** Use the correct getter/setter for the DataRef type

## License

This project is provided as-is for use with ProSim737 flight simulator.

## Support

For issues related to:
- **ProSimBridge**: Create an issue in this repository
- **ProSim737/ProSimSDK**: Contact ProSim support at https://prosim-ar.com

## Version History

### v1.0.0 (Current)
- Initial release with full vendor parity
- Complete DataRef lifecycle management
- Type-safe operations (int, double, bool, string, DateTime, RepositionData)
- Event-driven callback system
- Priority mode support
- Comprehensive error handling

## Acknowledgments

- ProSim737 team for the excellent flight simulator and SDK
- Vendor reference implementation for architectural guidance