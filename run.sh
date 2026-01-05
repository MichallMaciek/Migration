#!/bin/bash

# --- SETUP ---
if [ -z "$JAVA_HOME" ]; then
    if [ -x "/usr/libexec/java_home" ]; then
        export JAVA_HOME=$(/usr/libexec/java_home)
    else
        echo "Error: Set JAVA_HOME."
        exit 1
    fi
fi

# Detect OS
OS_NAME=$(uname -s)
if [ "$OS_NAME" == "Darwin" ]; then
    INCLUDE_PLATFORM="darwin"
    LIB_EXT="dylib"
else
    INCLUDE_PLATFORM="linux"
    LIB_EXT="so"
fi

# Clean and Create Build Directory
echo "--- CLEANING & PREPARING BUILD FOLDER ---"
rm -rf build
mkdir -p build

# --- COMPILE C++ LIB ---
echo "--- COMPILING C++ LIBRARY to build/ ---"
g++ -std=c++17 -fPIC -shared \
    -I"$JAVA_HOME/include" \
    -I"$JAVA_HOME/include/$INCLUDE_PLATFORM" \
    Game.cpp -o build/libmigration.$LIB_EXT

if [ $? -ne 0 ]; then echo "❌ C++ Lib Failed"; exit 1; fi

# --- COMPILE CONSOLE APP ---
echo "--- COMPILING CONSOLE APP to build/ ---"
g++ -std=c++17 -DCONSOLE_APP Game.cpp -o build/console_game

# --- COMPILE JAVA ---
echo "--- COMPILING JAVA to build/ ---"
javac -d build Migration.java

if [ $? -ne 0 ]; then echo "❌ Java Failed"; exit 1; fi

# --- RUN ---
echo "✅ SUCCESS. RUNNING GAME..."
cd build
java -Djava.library.path=. Migration