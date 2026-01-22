#!/bin/bash
# ProSimBridge Build Script for Unix/Linux/macOS
# Note: C++/CLI requires Windows and .NET Framework, but this script provides
# a framework for potential future cross-platform support or Wine-based builds

set -e

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "ProSimBridge Build Script"
echo "========================================"
echo ""

# Default values
BUILD_TYPE="Release"
BUILD_TESTS="ON"
CLEAN_BUILD=0
INSTALL_DIR="$(pwd)/install"
GENERATOR=""

# Parse command line arguments
show_help() {
    echo "Usage: ./build.sh [options]"
    echo ""
    echo "Options:"
    echo "  --debug              Build in Debug mode (default: Release)"
    echo "  --release            Build in Release mode"
    echo "  --no-tests           Don't build test executable"
    echo "  --clean              Clean build directory before building"
    echo "  --install-dir DIR    Set installation directory (default: ./install)"
    echo "  --generator GEN      Specify CMake generator"
    echo "  --help               Show this help message"
    echo ""
    echo "Examples:"
    echo "  ./build.sh                          Build in Release mode"
    echo "  ./build.sh --debug                  Build in Debug mode"
    echo "  ./build.sh --clean --release        Clean build and build in Release"
    echo "  ./build.sh --no-tests               Build without tests"
    echo ""
    echo "Note: ProSimBridge uses C++/CLI which requires Windows and .NET Framework."
    echo "      This script is provided for reference and future cross-platform work."
    echo ""
    exit 0
}

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --no-tests)
            BUILD_TESTS="OFF"
            shift
            ;;
        --clean)
            CLEAN_BUILD=1
            shift
            ;;
        --install-dir)
            INSTALL_DIR="$2"
            shift 2
            ;;
        --generator)
            GENERATOR="$2"
            shift 2
            ;;
        --help)
            show_help
            ;;
        *)
            echo -e "${RED}Unknown argument: $1${NC}"
            show_help
            ;;
    esac
done

# Display platform warning
echo -e "${YELLOW}WARNING: ProSimBridge uses C++/CLI and .NET Framework${NC}"
echo -e "${YELLOW}         This requires Windows and MSVC compiler${NC}"
echo ""

# Check platform
PLATFORM=$(uname -s)
case "$PLATFORM" in
    MINGW*|MSYS*|CYGWIN*)
        echo "Detected Windows environment: $PLATFORM"
        if [ -z "$GENERATOR" ]; then
            GENERATOR="Visual Studio 17 2022"
        fi
        ;;
    Darwin*)
        echo -e "${RED}ERROR: macOS is not supported for C++/CLI projects${NC}"
        echo "ProSimBridge requires Windows, Visual Studio, and .NET Framework"
        exit 1
        ;;
    Linux*)
        echo -e "${RED}ERROR: Linux is not directly supported for C++/CLI projects${NC}"
        echo "ProSimBridge requires Windows, Visual Studio, and .NET Framework"
        echo ""
        echo "Potential workarounds:"
        echo "  - Use Wine with Mono (experimental, not tested)"
        echo "  - Build on Windows and copy binaries"
        echo "  - Consider using Windows Subsystem for Linux (WSL) with Windows build tools"
        exit 1
        ;;
    *)
        echo -e "${RED}ERROR: Unknown platform: $PLATFORM${NC}"
        exit 1
        ;;
esac

# Display configuration
echo ""
echo "Configuration:"
echo "  Build Type:    $BUILD_TYPE"
echo "  Build Tests:   $BUILD_TESTS"
echo "  Generator:     $GENERATOR"
echo "  Install Dir:   $INSTALL_DIR"
echo "  Clean Build:   $CLEAN_BUILD"
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}ERROR: CMake not found in PATH${NC}"
    echo "Please install CMake 3.15 or later"
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
echo "Found CMake version: $CMAKE_VERSION"
echo ""

# Clean build directory if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    echo "Cleaning build directory..."
    rm -rf build
    echo "Done."
    echo ""
fi

# Create build directory
mkdir -p build

# Configure with CMake
echo "Configuring with CMake..."
cd build

CMAKE_ARGS=(
    ".."
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DCMAKE_INSTALL_PREFIX=$INSTALL_DIR"
    "-DBUILD_TESTS=$BUILD_TESTS"
)

if [ -n "$GENERATOR" ]; then
    CMAKE_ARGS+=("-G" "$GENERATOR" "-A" "x64")
fi

cmake "${CMAKE_ARGS[@]}"

if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: CMake configuration failed${NC}"
    cd ..
    exit 1
fi

echo ""
echo -e "${GREEN}Configuration successful!${NC}"
echo ""

# Build
echo "Building ProSimBridge..."
cmake --build . --config "$BUILD_TYPE" --parallel

if [ $? -ne 0 ]; then
    echo -e "${RED}ERROR: Build failed${NC}"
    cd ..
    exit 1
fi

echo ""
echo -e "${GREEN}Build successful!${NC}"
echo ""

# Optional: Run tests
if [ "$BUILD_TESTS" = "ON" ]; then
    echo "Running tests..."
    echo ""
    
    # Determine test executable path based on generator
    if [[ "$GENERATOR" == *"Visual Studio"* ]]; then
        TEST_EXE="bin/$BUILD_TYPE/ProSimBridgeTest.exe"
    else
        TEST_EXE="bin/ProSimBridgeTest.exe"
    fi
    
    if [ -f "$TEST_EXE" ]; then
        "$TEST_EXE"
        if [ $? -ne 0 ]; then
            echo -e "${YELLOW}WARNING: Some tests failed${NC}"
        else
            echo -e "${GREEN}All tests passed!${NC}"
        fi
        echo ""
    else
        echo -e "${YELLOW}WARNING: Test executable not found at $TEST_EXE${NC}"
        echo ""
    fi
fi

# Optional: Install
read -p "Do you want to install ProSimBridge? [y/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    echo "Installing ProSimBridge..."
    cmake --install . --config "$BUILD_TYPE"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}ERROR: Installation failed${NC}"
        cd ..
        exit 1
    fi
    
    echo ""
    echo -e "${GREEN}Installation successful!${NC}"
    echo "Installed to: $INSTALL_DIR"
    echo ""
fi

# Optional: Create package
read -p "Do you want to create a distribution package? [y/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    echo "Creating package..."
    cpack -C "$BUILD_TYPE"
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}ERROR: Package creation failed${NC}"
        cd ..
        exit 1
    fi
    
    echo ""
    echo -e "${GREEN}Package created successfully!${NC}"
    echo ""
fi

cd ..

echo "========================================"
echo -e "${GREEN}Build complete!${NC}"
echo "========================================"
echo ""

# Determine build output path
if [[ "$GENERATOR" == *"Visual Studio"* ]]; then
    BUILD_OUTPUT="build/bin/$BUILD_TYPE"
else
    BUILD_OUTPUT="build/bin"
fi

echo "Build artifacts located in: $BUILD_OUTPUT"
echo ""
echo "To run the test executable:"
echo "  cd $BUILD_OUTPUT"
echo "  ./ProSimBridgeTest.exe"
echo ""

exit 0