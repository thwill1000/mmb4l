#!/bin/bash
set -e

PROJECT_NAME=${1:-MmBasicForAndroid}
PACKAGE_NAME=${2:-com.sockpuppetstudios.mmb4a}

echo "📱 Installing and running $PROJECT_NAME on Android Emulator..."

# Check if emulator is running
if ! adb devices | grep -q "emulator"; then
    echo "❌ No emulator detected. Start the emulator first with: start-emulator.sh"
    exit 1
fi

# Check if APK exists
APK_PATH="$PROJECT_NAME/app/build/outputs/apk/debug/app-debug.apk"
if [ ! -f "$APK_PATH" ]; then
    echo "❌ APK not found at $APK_PATH"
    echo "Build your project first with: build-example.sh $PROJECT_NAME $PACKAGE_NAME"
    exit 1
fi

echo "🔧 Installing APK on emulator..."
adb install -r "$APK_PATH"

if [ $? -eq 0 ]; then
    echo "✅ APK installed successfully!"
    
    echo "🚀 Starting the app..."
    adb shell am start -n "$PACKAGE_NAME/.MainActivity"
    
    echo "📱 App should now be running on the emulator!"
    echo ""
    echo "📋 Monitor app logs with:"
    echo "   adb logcat -s SDL2App"
    echo ""
    echo "🎮 App features:"
    echo "   - Blue background (OpenGL ES rendering)"
    echo "   - Touch/click input handling"
    echo "   - SDL2 event loop running at ~60 FPS"
    echo ""
    echo "🐛 If you encounter issues:"
    echo "   adb logcat | grep -E '(SDL|AndroidRuntime|FATAL)'"
    
    # Start monitoring logs in the background
    echo ""
    echo "📊 Starting log monitor (Ctrl+C to stop)..."
    adb logcat -s SDL2App &
    LOG_PID=$!
    
    echo "Press Enter to stop log monitoring..."
    read
    kill $LOG_PID 2>/dev/null || true
    
else
    echo "❌ Failed to install APK"
    exit 1
fi
