#!/bin/bash
set -e

AVD_NAME="SDL2_Test_AVD"

echo "🚀 Starting Android Emulator in headless mode (no GUI)..."

# Check if emulator exists
if ! command -v emulator >/dev/null 2>&1; then
    echo "❌ Emulator not found. Make sure Android SDK is properly installed."
    exit 1
fi

# Check if AVD exists
if ! avdmanager list avd | grep -q "$AVD_NAME"; then
    echo "❌ AVD '$AVD_NAME' not found. Run setup-emulator.sh first."
    echo ""
    echo "📋 Available AVDs:"
    avdmanager list avd
    exit 1
fi

# Kill any existing emulator processes
echo "🧹 Cleaning up any existing emulator processes..."
pkill -f "emulator.*$AVD_NAME" || true
pkill -f "qemu-system" || true
sleep 3

echo "🔧 Starting headless emulator: $AVD_NAME"
echo "⚙️  Configuration:"
echo "   - Mode: Headless (no GUI window)"
echo "   - GPU: Software rendering (off)"
echo "   - Audio: Disabled"
echo "   - Memory: 2048MB"
echo "   - Network: Full speed"
echo ""

# Start emulator in headless mode with optimal settings for Docker
emulator -avd "$AVD_NAME" \
    -no-window \
    -no-audio \
    -no-boot-anim \
    -gpu off \
    -memory 2048 \
    -cores 2 \
    -netdelay none \
    -netspeed full \
    -no-snapshot-save \
    -no-snapshot-load \
    -no-metrics \
    -verbose &

EMULATOR_PID=$!
echo "📱 Headless emulator starting with PID: $EMULATOR_PID"

# Function to cleanup on exit
cleanup() {
    echo ""
    echo "🛑 Stopping emulator..."
    kill $EMULATOR_PID 2>/dev/null || true
    pkill -f "emulator.*$AVD_NAME" || true
    pkill -f "qemu-system" || true
    # Give processes time to clean up
    sleep 2
    echo "✅ Emulator stopped"
}
trap cleanup EXIT INT TERM

echo "⏳ Waiting for emulator to boot (this may take 2-5 minutes)..."
echo "    The emulator runs without a GUI window - you'll interact via ADB"

# Wait for emulator to be ready
timeout=600  # 10 minutes timeout
elapsed=0
boot_completed=false

while [ $elapsed -lt $timeout ]; do
    # Check if emulator process is still running
    if ! kill -0 $EMULATOR_PID 2>/dev/null; then
        echo "❌ Emulator process died unexpectedly"
        echo ""
        echo "🔍 Troubleshooting:"
        echo "   - The emulator may need more time to initialize"
        echo "   - Check system resources (RAM, CPU)"
        echo "   - Try recreating the AVD: setup-emulator.sh"
        exit 1
    fi
    
    # Check if device is detected by ADB
    if adb devices 2>/dev/null | grep -q "emulator.*device"; then
        # Check if boot is completed
        boot_status=$(adb shell getprop sys.boot_completed 2>/dev/null || echo "0")
        if [ "$boot_status" = "1" ]; then
            boot_completed=true
            break
        fi
    elif adb devices 2>/dev/null | grep -q "emulator.*offline"; then
        echo "📱 Emulator detected but still booting..."
    fi
    
    sleep 5
    elapsed=$((elapsed + 5))
    
    # Show progress every 30 seconds
    if [ $((elapsed % 30)) -eq 0 ]; then
        echo "⏳ Still waiting... ($elapsed/$timeout seconds)"
        device_status=$(adb devices 2>/dev/null | grep emulator | head -1 || echo 'No emulator detected yet')
        echo "   Current status: $device_status"
        
        # Show some system info to confirm it's working
        if adb devices 2>/dev/null | grep -q "emulator"; then
            boot_progress=$(adb shell getprop sys.boot_completed 2>/dev/null || echo "unknown")
            echo "   Boot completed: $boot_progress"
        fi
    fi
done

if [ "$boot_completed" = true ]; then
    echo ""
    echo "✅ Headless emulator is ready!"
    
    # Wait a bit more for services to start
    echo "🎨 Waiting for Android services to start..."
    sleep 15
    
    echo ""
    echo "🎉 Android Emulator is now running in headless mode!"
    echo ""
    echo "📱 Device Information:"
    adb shell getprop ro.build.version.release 2>/dev/null | sed 's/^/   Android Version: /' || echo "   Android Version: Unknown"
    adb shell getprop ro.product.model 2>/dev/null | sed 's/^/   Device Model: /' || echo "   Device Model: Unknown"
    adb shell getprop ro.build.version.sdk 2>/dev/null | sed 's/^/   API Level: /' || echo "   API Level: Unknown"
    
    echo ""
    echo "📋 Essential ADB commands:"
    echo "   adb devices                           # List connected devices"
    echo "   adb shell                             # Access emulator shell"
    echo "   adb install app-debug.apk             # Install an APK"
    echo "   adb shell am start -n PACKAGE/.MainActivity  # Start an app"
    echo "   adb logcat                            # View system logs"
    echo "   adb logcat -s SDL2App                 # View your app logs"
    echo "   adb emu kill                          # Stop emulator"
    echo ""
    echo "🎮 Perfect for SDL2 app development and testing!"
    echo ""
    echo "📋 Connected devices:"
    adb devices
    echo ""
    echo "💡 The emulator is running without a GUI - all interaction is via ADB"
    echo "⌨️  Press Ctrl+C to stop the emulator"
    echo ""
    
    # Test basic functionality
    echo "🧪 Testing basic functionality..."
    if adb shell echo "Hello Android" >/dev/null 2>&1; then
        echo "✅ Shell access: Working"
    else
        echo "⚠️  Shell access: Limited"
    fi
    
    if adb shell pm list packages | head -5 >/dev/null 2>&1; then
        echo "✅ Package manager: Working"
    else
        echo "⚠️  Package manager: Limited"
    fi
    
    echo ""
    echo "🎯 Ready to install and test your SDL2 applications!"
    
    # Keep the script running and show periodic status
    while true; do
        sleep 60
        if ! kill -0 $EMULATOR_PID 2>/dev/null; then
            echo "❌ Emulator process has stopped"
            break
        fi
        # Optional: show a brief status every few minutes
        # echo "⏰ $(date): Emulator still running (PID: $EMULATOR_PID)"
    done
    
else
    echo ""
    echo "❌ Emulator failed to start within $timeout seconds"
    echo ""
    echo "🔍 Troubleshooting steps:"
    echo "   1. Check emulator logs: emulator -avd $AVD_NAME -verbose -no-window"
    echo "   2. Verify AVD exists: avdmanager list avd"
    echo "   3. Check system resources: free -h && df -h"
    echo "   4. Try recreating AVD: setup-emulator.sh"
    echo ""
    echo "🔧 Alternative commands to try:"
    echo "   emulator -avd $AVD_NAME -no-window -no-audio -gpu off -verbose"
    echo "   emulator -avd $AVD_NAME -no-window -memory 1024"
    
    exit 1
fi
