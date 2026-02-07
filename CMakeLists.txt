cmake_minimum_required(VERSION 3.15)

# Allow version to be overridden via -DPROSIMBRIDGE_VERSION=x.y.z (e.g. from CI tag)
if(NOT DEFINED PROSIMBRIDGE_VERSION)
    set(PROSIMBRIDGE_VERSION "0.0.0")
endif()

project(ProSimBridge VERSION ${PROSIMBRIDGE_VERSION} LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable CLR support for C++/CLI
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clr")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Define the library
add_library(ProSimBridge SHARED
    ProSimBridge.cpp
    ProSimBridge.h
    ManagedWrapper.cpp
    ManagedWrapper.h
    AssemblyInfo.cpp
    pch.cpp
    pch.h
    Resource.h
    app.rc
)

# Set precompiled header
target_precompile_headers(ProSimBridge PRIVATE pch.h)

# Add preprocessor definitions
target_compile_definitions(ProSimBridge PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    UNICODE
    _UNICODE
)

# Set warning level
target_compile_options(ProSimBridge PRIVATE /W3)

# Add .NET Framework references
set_target_properties(ProSimBridge PROPERTIES
    VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2"
    VS_DOTNET_REFERENCES "System;System.Data;System.Xml"
    COMMON_LANGUAGE_RUNTIME ""
)

# Add ProSimSDK reference
set_target_properties(ProSimBridge PROPERTIES
    VS_DOTNET_REFERENCE_ProSimSDK "${CMAKE_CURRENT_SOURCE_DIR}/libs/ProSimSDK.dll"
)

# Include directories
target_include_directories(ProSimBridge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link directories
target_link_directories(ProSimBridge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

# Post-build: Copy ProSimSDK.dll to output directory
add_custom_command(TARGET ProSimBridge POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/ProSimSDK.dll"
        "$<TARGET_FILE_DIR:ProSimBridge>/ProSimSDK.dll"
    COMMENT "Copying ProSimSDK.dll to output directory"
)

# Installation rules
install(TARGETS ProSimBridge
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES
    ProSimBridge.h
    DESTINATION include
)

install(FILES
    libs/ProSimSDK.dll
    DESTINATION bin
)

install(FILES
    README.md
    CHANGELOG.md
    DESTINATION docs
)

# Build test executable (optional)
option(BUILD_TESTS "Build test executable" ON)

if(BUILD_TESTS)
    add_executable(ProSimBridgeTest test.cpp)
    
    target_include_directories(ProSimBridgeTest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    target_link_libraries(ProSimBridgeTest PRIVATE ProSimBridge)
    
    # Copy dependencies for test
    add_custom_command(TARGET ProSimBridgeTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:ProSimBridge>"
            "$<TARGET_FILE_DIR:ProSimBridgeTest>/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/libs/ProSimSDK.dll"
            "$<TARGET_FILE_DIR:ProSimBridgeTest>/ProSimSDK.dll"
        COMMENT "Copying dependencies for test"
    )
    
    install(TARGETS ProSimBridgeTest
        RUNTIME DESTINATION bin
    )
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "ProSimBridge")
set(CPACK_PACKAGE_VENDOR "ProSimBridge")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Native C API bridge for ProSim737 SDK")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "ProSimBridge")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Set generator based on platform
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
endif()

include(CPack)

# Print configuration summary
message(STATUS "")
message(STATUS "ProSimBridge Configuration Summary")
message(STATUS "===================================")
message(STATUS "Version:          ${PROJECT_VERSION}")
message(STATUS "Build Type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(STATUS "Install Prefix:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Build Tests:      ${BUILD_TESTS}")
message(STATUS "")