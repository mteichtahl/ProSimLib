@echo off
REM ProSimBridge Build Script for Windows
REM Requires: CMake 3.15+, Visual Studio 2019+ with C++/CLI support

setlocal enabledelayedexpansion

echo ========================================
echo ProSimBridge Build Script
echo ========================================
echo.

REM Parse command line arguments
set BUILD_TYPE=Release
set BUILD_TESTS=ON
set CLEAN_BUILD=0
set INSTALL_DIR=%CD%\install
set GENERATOR="Visual Studio 17 2022"

:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if /i "%~1"=="--release" (
    set BUILD_TYPE=Release
    shift
    goto parse_args
)
if /i "%~1"=="--no-tests" (
    set BUILD_TESTS=OFF
    shift
    goto parse_args
)
if /i "%~1"=="--clean" (
    set CLEAN_BUILD=1
    shift
    goto parse_args
)
if /i "%~1"=="--install-dir" (
    set INSTALL_DIR=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--vs2019" (
    set GENERATOR="Visual Studio 16 2019"
    shift
    goto parse_args
)
if /i "%~1"=="--vs2022" (
    set GENERATOR="Visual Studio 17 2022"
    shift
    goto parse_args
)
if /i "%~1"=="--help" (
    goto show_help
)
echo Unknown argument: %~1
goto show_help

:end_parse

REM Display configuration
echo Configuration:
echo   Build Type:    %BUILD_TYPE%
echo   Build Tests:   %BUILD_TESTS%
echo   Generator:     %GENERATOR%
echo   Install Dir:   %INSTALL_DIR%
echo   Clean Build:   %CLEAN_BUILD%
echo.

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake not found in PATH
    echo Please install CMake 3.15 or later
    exit /b 1
)

REM Clean build directory if requested
if %CLEAN_BUILD%==1 (
    echo Cleaning build directory...
    if exist build (
        rmdir /s /q build
    )
    echo Done.
    echo.
)

REM Create build directory
if not exist build (
    mkdir build
)

REM Configure with CMake
echo Configuring with CMake...
cd build
cmake .. ^
    -G %GENERATOR% ^
    -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -DBUILD_TESTS=%BUILD_TESTS%

if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed
    cd ..
    exit /b 1
)

echo.
echo Configuration successful!
echo.

REM Build
echo Building ProSimBridge...
cmake --build . --config %BUILD_TYPE% --parallel

if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed
    cd ..
    exit /b 1
)

echo.
echo Build successful!
echo.

REM Optional: Run tests
if "%BUILD_TESTS%"=="ON" (
    echo Running tests...
    echo.
    
    REM Run the test executable
    if exist bin\%BUILD_TYPE%\ProSimBridgeTest.exe (
        bin\%BUILD_TYPE%\ProSimBridgeTest.exe
        if !ERRORLEVEL! neq 0 (
            echo WARNING: Some tests failed
        ) else (
            echo All tests passed!
        )
        echo.
    ) else (
        echo WARNING: Test executable not found
        echo.
    )
)

REM Optional: Install
choice /C YN /M "Do you want to install ProSimBridge"
if %ERRORLEVEL%==1 (
    echo.
    echo Installing ProSimBridge...
    cmake --install . --config %BUILD_TYPE%
    
    if !ERRORLEVEL! neq 0 (
        echo ERROR: Installation failed
        cd ..
        exit /b 1
    )
    
    echo.
    echo Installation successful!
    echo Installed to: %INSTALL_DIR%
    echo.
)

REM Optional: Create package
choice /C YN /M "Do you want to create a distribution package"
if %ERRORLEVEL%==1 (
    echo.
    echo Creating package...
    cpack -C %BUILD_TYPE%
    
    if !ERRORLEVEL! neq 0 (
        echo ERROR: Package creation failed
        cd ..
        exit /b 1
    )
    
    echo.
    echo Package created successfully!
    echo.
)

cd ..

echo ========================================
echo Build complete!
echo ========================================
echo.
echo Build artifacts located in: build\bin\%BUILD_TYPE%
echo.
echo To run the test executable:
echo   cd build\bin\%BUILD_TYPE%
echo   ProSimBridgeTest.exe
echo.

exit /b 0

:show_help
echo Usage: build.bat [options]
echo.
echo Options:
echo   --debug              Build in Debug mode (default: Release)
echo   --release            Build in Release mode
echo   --no-tests           Don't build test executable
echo   --clean              Clean build directory before building
echo   --install-dir DIR    Set installation directory (default: .\install)
echo   --vs2019             Use Visual Studio 2019 generator
echo   --vs2022             Use Visual Studio 2022 generator (default)
echo   --help               Show this help message
echo.
echo Examples:
echo   build.bat                           Build in Release mode
echo   build.bat --debug                   Build in Debug mode
echo   build.bat --clean --release         Clean build and build in Release
echo   build.bat --no-tests --vs2019       Build without tests using VS2019
echo.
exit /b 0