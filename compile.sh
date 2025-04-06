#!/bin/bash

# Check if cmake and ninja are installed
if ! command -v cmake &> /dev/null; then
    echo "Error: cmake is not installed. Please install cmake."
    exit 1
fi

if ! command -v ninja &> /dev/null; then
    echo "Error: ninja is not installed. Please install ninja."
    exit 1
fi

# Check if gcc-10 and g++-10 are installed
if ! command -v gcc-10 &> /dev/null || ! command -v g++-10 &> /dev/null; then
    echo "Error: gcc-10 or g++-10 is not installed. Please install gcc-10 and g++-10."
    exit 1
fi

# Run cmake to generate build files
/usr/bin/cmake --no-warn-unused-cli \
    -DCMAKE_BUILD_TYPE:STRING=Release \
    -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
    -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc-10 \
    -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++-10 \
    -S/project/src \
    -B/project/build \
    -G Ninja

# Build the project using ninja
/usr/bin/cmake --build /project/build --config Release --target all

mkdir /project/bin
cp /project/build/fastibs /project/bin
cp /project/build/fastibsmapper /project/bin
cp /project/build/KDBIntersect /project/bin

echo "Build completed successfully."
