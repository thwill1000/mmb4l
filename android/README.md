# Android NDK + SDL2 Development Environment

This Docker container provides a complete environment for Android development with NDK and SDL2.

## What's Included
- Ubuntu 22.04 base
- Android SDK with API level 33
- Android NDK r25c
- SDL2 2.28.5
- CMake and Ninja build system
- Gradle 8.0

## Quick Start

### 1. Create a new SDL2 Android project:
```bash
build-android-sdl2.sh MyGameApp com.mygame.app
```

### 2. Build the project:
```bash
cd MyGameApp
./gradlew assembleDebug
```

### 3. The APK will be generated in:
```
app/build/outputs/apk/debug/app-debug.apk
```

## Directory Structure
- `/opt/android-sdk` - Android SDK
- `/opt/android-ndk` - Android NDK
- `/opt/sdl2/SDL2` - SDL2 source code
- `/opt/sdl2/android-template` - SDL2 Android project template
- `/workspace` - Your working directory

## Environment Variables
- `ANDROID_HOME=/opt/android-sdk`
- `ANDROID_NDK_HOME=/opt/android-ndk`
- `PATH` includes all necessary tool paths

## Example Files
- `CMakeLists.txt.example` - Example CMake configuration
- `main.cpp.example` - Example SDL2 application

## Building Custom Native Code
1. Place your C/C++ source files in `app/src/main/cpp/`
2. Update `app/src/main/cpp/CMakeLists.txt`
3. Build with `./gradlew assembleDebug`

## Connecting a Device
To deploy to a real device, you'll need to:
1. Enable USB debugging on your Android device
2. Run the container with USB access: `docker run --privileged -v /dev/bus/usb:/dev/bus/usb`
3. Use `adb devices` to verify connection
