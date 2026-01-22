# Changelog

All notable changes to ProSimBridge will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-01-21

### Added - Phase 1: Error Handling Foundation
- Comprehensive error code system (8 error codes)
- Thread-local error message storage
- `ProSim_GetLastError()` function for detailed error messages
- Error handling in all API functions with detailed exception messages
- Helper functions `SetLastError()` and `StoreExceptionMessage()`

### Changed - Phase 2: Architecture Refactoring
- Migrated from GCHandle to gcroot<T> for managed object lifetime management
- Implemented RAII pattern for resource cleanup
- Created `ProSimConnectWrapper` class with proper constructor/destructor
- Created `DataRefWrapper` class with proper constructor/destructor
- Added `ConnectionEventBridge` and `DataRefEventBridge` for native-to-managed callbacks
- Implemented automatic event subscription/unsubscription
- Added disposal flags to prevent double-free scenarios

### Added - Phase 3: DataRef Enhancement
- Complete DataRef lifecycle management:
  - `DataRef_Create()` - Create DataRef instances
  - `DataRef_Destroy()` - Clean up DataRef resources
  - `DataRef_Register()` - Manual registration support
  - `DataRef_GetName()` - Retrieve DataRef name
- Type-specific getter functions:
  - `DataRef_GetInt()` - Read as 32-bit integer
  - `DataRef_GetDouble()` - Read as double
  - `DataRef_GetBool()` - Read as boolean
  - `DataRef_GetString()` - Read as string
- Type-specific setter functions:
  - `DataRef_SetInt()` - Write 32-bit integer
  - `DataRef_SetDouble()` - Write double
  - `DataRef_SetBool()` - Write boolean
  - `DataRef_SetString()` - Write string
- Support for delayed registration with `register_now` parameter

### Added - Phase 4: Callback System
- Callback function pointer types:
  - `ConnectionCallback` - Connection state changes
  - `DataRefChangeCallback` - DataRef value changes
- Connection callback functions:
  - `ProSim_SetOnConnect()` - Register connect callback
  - `ProSim_SetOnDisconnect()` - Register disconnect callback
- DataRef callback function:
  - `DataRef_SetOnDataChange()` - Register value change callback
- Native callback storage in wrapper classes
- User data context passing (void* userData)
- Automatic callback invocation on events

### Added - Phase 5: Advanced Features
- Data structures:
  - `DateTime` - Year, month, day, hour, minute, second, millisecond
  - `RepositionData` - Complete aircraft positioning with 8 fields
- Advanced DataRef operations:
  - `DataRef_GetDateTime()` - Read DateTime values
  - `DataRef_SetDateTime()` - Write DateTime values
  - `DataRef_SetReposition()` - Set aircraft position and state
- Advanced connection operations:
  - `ProSim_SetPriorityMode()` - Enable/disable SDK priority mode
- Full implementation in ManagedWrapper layer
- Comprehensive error handling for all advanced features

### Added - Phase 7: Testing & Documentation
- Comprehensive test suite in `test.cpp`:
  - Legacy API tests (Phase 1-2)
  - DataRef lifecycle tests (Phase 3)
  - Callback system tests (Phase 4)
  - Advanced features tests (Phase 5)
- Complete documentation:
  - README.md with API reference and examples
  - CHANGELOG.md for version tracking
  - API_REFERENCE.md for detailed function documentation
  - Inline code documentation

## [1.6.0] - 2026-01-21

### Added - Phase 6: Build Configuration
- CMake build system support with CMakeLists.txt
  - Cross-platform build configuration
  - Automatic dependency copying (ProSimSDK.dll)
  - Optional test executable building
  - Installation rules with proper file layout
  - CPack integration for package creation
- Automated build scripts:
  - `build.bat` - Windows build automation with options
  - `build.sh` - Unix reference script (Windows required)
- Build script features:
  - Multiple build configurations (Debug/Release)
  - Clean build support
  - Custom installation directory
  - Visual Studio version selection (2019/2022)
  - Automatic test execution
  - Interactive install and package creation
- CMake configuration options:
  - BUILD_TESTS - Enable/disable test building
  - CMAKE_INSTALL_PREFIX - Custom install location
  - CMAKE_BUILD_TYPE - Build configuration
- Package generation support:
  - ZIP archives for distribution
  - NSIS installer for Windows
  - Proper versioning (1.6.0)
- Build output organization:
  - Structured bin/lib directories
  - Automatic dependency management
  - Parallel build support

## [Unreleased]

### Future Enhancements
- Performance profiling and optimization
- Additional DataRef type support as needed
- Extended error recovery mechanisms

## Version History Summary

### v1.0.0 - Full Vendor Parity Release
This release achieves complete parity with the vendor's reference implementation:
- ✅ Error handling with detailed messages
- ✅ Modern C++/CLI architecture with gcroot<T>
- ✅ Type-safe DataRef operations (6 types)
- ✅ Event-driven callback system
- ✅ Advanced features (DateTime, RepositionData, Priority Mode)
- ✅ Comprehensive documentation and tests

## Migration Guide

### From GCHandle to gcroot<T> (Phase 2)
If you were using an earlier version with GCHandle:
- No API changes required
- Memory management is now automatic
- Better exception safety
- Improved performance

### From Generic DataRef to Type-Specific (Phase 3)
Old approach:
```cpp
double value;
ProSim_ReadDataRef(prosim, "Aircraft.Altitude", &value);
```

New approach (recommended):
```cpp
DataRefHandle alt = DataRef_Create("Aircraft.Altitude", 100, prosim, true);
int32_t value;
DataRef_GetInt(alt, &value);
DataRef_Destroy(alt);
```

Benefits:
- Type safety
- Better performance
- Lifecycle control
- Event notifications

## Known Issues

None at this time.

## Breaking Changes

None - This is the initial release.

## Contributors

- Initial implementation based on vendor reference architecture
- Full parity implementation across 7 phases

## Support

For issues and questions:
- GitHub Issues: [Create an issue](https://github.com/mteichtahl/ProSimLib/issues)
- ProSim Support: https://prosim-ar.com

---

[1.0.0]: https://github.com/mteichtahl/ProSimLib/releases/tag/v1.0.0