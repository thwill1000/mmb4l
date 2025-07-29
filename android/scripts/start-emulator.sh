#!/bin/bash
set -e

AVD_NAME="SDL2_Test_AVD"

echo "🚀 Starting Android Emulator..."

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
sleep 2

echo "🔧 Starting emulator: $AVD_NAME"
echo "⚙️  Using configuration:"
echo "   - GPU acceleration: host"
echo "   - Memory: 2048MB"
echo "   - Network: full speed, no latency"
echo "   - Audio: enabled"
echo ""

# Start emulator with optimized settings
emulator -avd "$AVD_NAME" \
    -gpu host \
    -memory 2048 \
    -cores 4 \
    -netdelay none \
    -netspeed full \
    -no-snapshot-save \
    -no-snapshot-load \
    -camera-back webcam0 \
    -camera-front webcam0 \
    -skin 1080x2280 \
    -verbose \
    &

EMULATOR_PID=$!
echo "📱 Emulator starting with PID: $EMULATOR_PID"

# Function to cleanup on exit
cleanup() {
    echo ""
    echo "🛑 Stopping emulator..."
    kill $EMULATOR_PID 2>/dev/null || true
    pkill -f "emulator.*$AVD_NAME" || true
}
trap cleanup EXIT INT TERM

echo "⏳ Waiting for emulator to boot (this may take 2-5 minutes)..."
echo "    You can monitor progress with: adb logcat | grep Boot"

# Wait for emulator to be ready
timeout=600  # 10 minutes timeout
elapsed=0
boot_completed=false

while [ $elapsed -lt $timeout ]; do
    # Check if emulator process is still running
    if ! kill -0 $EMULATOR_PID 2>/dev/null; then
        echo "❌ Emulator process died unexpectedly"
        exit 1
    fi
    
    # Check if device is detected
    if adb devices | grep -q "emulator.*device"; then
        # Check if boot is completed
        boot_status=$(adb shell getprop sys.boot_completed 2>/dev/null || echo "0")
        if [ "$boot_status" = "1" ]; then
            boot_completed=true
            break
        fi
    fi
    
    sleep 5
    elapsed=$((elapsed + 5))
    
    # Show progress every 30 seconds
    if [ $((elapsed % 30)) -eq 0 ]; then
        echo "⏳ Still waiting... ($elapsed/$timeout seconds)"
        echo "   Current status: $(adb devices | grep emulator | head -1 || echo 'No emulator detected yet')"
    fi
done

if [ "$boot_completed" = true ]; then
    echo ""
    echo "✅ Emulator is ready!"
    
    # Wait a bit more for UI to be fully loaded
    echo "🎨 Waiting for UI to fully load..."
    sleep 10
    
    echo ""
    echo "🎉 Android Emulator is now running!"
    echo ""
    echo "📱 Device Information:"
    adb shell getprop ro.build.version.release 2>/dev/null | sed 's/^/   Android Version: /' || true
    adb shell getprop ro.product.model 2>/dev/null | sed 's/^/   Device Model: /' || true
    adb shell getprop ro.build.version.sdk 2>/dev/null | sed 's/^/   API Level: /' || true
    echo ""
    echo "📋 Useful commands:"
    echo "   adb devices                           # List connected devices"
    echo "   adb shell                             # Access emulator shell"
    echo "   adb install app-debug.apk             # Install an APK"
    echo "   adb logcat                            # View system logs"
    echo "   adb logcat -s SDL2App                 # View your app logs"
    echo "   adb emu kill                          # Stop emulator"
    echo ""
    echo "🎮 Ready for SDL2 app development!"
    echo ""
    
    # Show connected devices
    echo "📱 Connected devices:"
    adb devices
    echo ""
    echo "⌨️  Press Ctrl+C to stop the emulator"
    
    # Keep the script running
    wait
else
    echo ""
    echo "❌ Emulator failed to start within $timeout seconds"
    echo ""
    echo "🔍 Troubleshooting:"
    echo "   1. Check if virtualization is enabled in your host system"
    echo "   2. Verify /dev/kvm is available (for Linux)"
    echo "   3. Try starting manually: emulator -avd $AVD_NAME -verbose"
    echo "   4. Check logs: emulator -avd $AVD_NAME -verbose -show-kernel"
    echo ""
    echo "🔧 Alternative start commands:"
    echo "   emulator -avd $AVD_NAME -no-audio -no-window    # Headless mode"
    echo "   emulator -avd $AVD_NAME -gpu swiftshader_indirect # Software rendering"
    
    exit 1
fi
