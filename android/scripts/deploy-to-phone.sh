#!/bin/bash
set -e

cd "projects"

PROJECT_NAME=${1:-MmBasicForAndroid}
PACKAGE_NAME=${2:-com.sockpuppetstudios.mmb4a}

echo "📱 Deploying $PROJECT_NAME to Android Device"
echo "=============================================="

# Check if device is connected
if ! adb devices | grep -v "List of devices" | grep -q "device"; then
    echo "❌ No Android device detected!"
    echo ""
    echo "🔧 Troubleshooting:"
    echo "   1. Connect via USB and enable USB debugging"
    echo "   2. Or set up wireless ADB with: setup-wireless-adb.sh"
    echo "   3. Check connection with: adb devices"
    exit 1
fi

echo "✅ Device detected:"
adb devices | grep device

# Check if APK exists
APK_PATH="$PROJECT_NAME/app/build/outputs/apk/debug/app-debug.apk"
if [ ! -f "$APK_PATH" ]; then
    echo "❌ APK not found at: $APK_PATH"
    echo ""
    echo "🔨 Build your project first:"
    echo "   build-example.sh $PROJECT_NAME $PACKAGE_NAME"
    exit 1
fi

echo ""
echo "📦 APK found: $APK_PATH"
echo "📊 APK size: $(du -h "$APK_PATH" | cut -f1)"

# Get device info
echo ""
echo "📱 Target device information:"
DEVICE_MODEL=$(adb shell getprop ro.product.model 2>/dev/null || echo "Unknown")
ANDROID_VERSION=$(adb shell getprop ro.build.version.release 2>/dev/null || echo "Unknown")
API_LEVEL=$(adb shell getprop ro.build.version.sdk 2>/dev/null || echo "Unknown")

echo "   Model: $DEVICE_MODEL"
echo "   Android: $ANDROID_VERSION (API $API_LEVEL)"

# Check if app is already installed
if adb shell pm list packages | grep -q "$PACKAGE_NAME"; then
    echo ""
    echo "🔄 App already installed, performing update..."
    INSTALL_FLAG="-r"
else
    echo ""
    echo "📥 Installing new app..."
    INSTALL_FLAG=""
fi

# Install the APK
echo "⏳ Installing APK..."
if adb install $INSTALL_FLAG "$APK_PATH"; then
    echo "✅ Installation successful!"
else
    echo "❌ Installation failed!"
    echo ""
    echo "🔍 Common solutions:"
    echo "   1. Enable 'Install via USB' in Developer Options"
    echo "   2. Disable 'Play Protect' temporarily"
    echo "   3. Try uninstalling first: adb uninstall $PACKAGE_NAME"
    exit 1
fi

echo ""
echo "🚀 Launching app..."
if adb shell am start -n "$PACKAGE_NAME/.MainActivity"; then
    echo "✅ App launched successfully!"
    echo ""
    echo "🎮 Your SDL2 app should now be running on your device!"
    echo ""
    echo "📋 Useful commands:"
    echo "   View app logs:     adb logcat -s MMB4A"
    echo "   Stop app:          adb shell am force-stop $PACKAGE_NAME"
    echo "   Uninstall app:     adb uninstall $PACKAGE_NAME"
    echo "   Take screenshot:   adb exec-out screencap -p > screenshot.png"
    echo ""
    echo "🔍 Monitor your app:"
    echo "   adb logcat -s MMB4A"
    
    # Offer to start log monitoring
    echo ""
    echo "📊 Start log monitoring? (y/n)"
    read -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "📱 Monitoring logs (Ctrl+C to stop)..."
        adb logcat -s MMB4A
    fi
    
else
    echo "❌ Failed to launch app"
    echo ""
    echo "🔍 Try launching manually:"
    echo "   1. Look for '$PROJECT_NAME' icon on your device"
    echo "   2. Or use: adb shell monkey -p $PACKAGE_NAME -c android.intent.category.LAUNCHER 1"
fi
