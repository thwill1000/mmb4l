#!/bin/bash
set -e

echo "🚀 Setting up Android Emulator..."

# Check if emulator is available
if ! command -v emulator >/dev/null 2>&1; then
    echo "❌ Emulator not found. Run fix-emulator.sh first."
    exit 1
fi

# Check available skins
echo "🎨 Checking available skins..."
AVAILABLE_SKINS=$(emulator -list-skins 2>/dev/null || echo "")
echo "Available skins: $AVAILABLE_SKINS"

# Choose a simple skin that should be available
if echo "$AVAILABLE_SKINS" | grep -q "1080x1920"; then
    SKIN="1080x1920"
elif echo "$AVAILABLE_SKINS" | grep -q "768x1280"; then
    SKIN="768x1280"
else
    SKIN="480x800"  # Fallback to basic resolution
fi

echo "📱 Using skin: $SKIN"

# Check what system images are available
echo "📋 Available system images:"
avdmanager list target

# Check if our target system image is installed
if ! avdmanager list target | grep -q "android-33"; then
    echo "📦 Installing required system image..."
    sdkmanager "system-images;android-33;google_apis;x86_64"
fi

# Accept any additional licenses
yes | sdkmanager --licenses >/dev/null 2>&1 || true

# Create AVD (Android Virtual Device)
echo "🔧 Creating Android Virtual Device..."

AVD_NAME="SDL2_Test_AVD"

# Remove existing AVD if it exists
if avdmanager list avd | grep -q "$AVD_NAME"; then
    echo "🗑️  Removing existing AVD..."
    avdmanager delete avd -n "$AVD_NAME"
fi

# Create new AVD with basic device profile
echo "📱 Creating new AVD: $AVD_NAME"
echo "no" | avdmanager create avd \
    --force \
    --name "$AVD_NAME" \
    --package "system-images;android-33;google_apis;x86_64" \
    --tag "google_apis" \
    --abi "x86_64" \
    --device "pixel"

# Configure AVD for better performance
AVD_DIR="$HOME/.android/avd/${AVD_NAME}.avd"
if [ -d "$AVD_DIR" ]; then
    echo "⚙️  Configuring AVD for optimal performance..."
    
    # Create config.ini with performance optimizations and simple skin
    cat > "$AVD_DIR/config.ini" << EOF
AvdId=$AVD_NAME
PlayStore.enabled=false
abi.type=x86_64
avd.ini.displayname=$AVD_NAME
avd.ini.encoding=UTF-8
disk.dataPartition.size=2048MB
fastboot.chosenSnapshotFile=
fastboot.forceChosenSnapshotBoot=no
fastboot.forceColdBoot=no
fastboot.forceFastBoot=yes
hw.accelerometer=yes
hw.audioInput=yes
hw.audioOutput=yes
hw.battery=yes
hw.camera.back=none
hw.camera.front=none
hw.cpu.arch=x86_64
hw.cpu.ncore=4
hw.dPad=no
hw.device.manufacturer=Google
hw.device.name=Nexus 5X
hw.gps=yes
hw.gpu.enabled=yes
hw.gpu.mode=host
hw.initialOrientation=Portrait
hw.keyboard=yes
hw.lcd.density=420
hw.lcd.height=1920
hw.lcd.width=1080
hw.mainKeys=no
hw.ramSize=2048
hw.sdCard=yes
hw.sensors.orientation=yes
hw.sensors.proximity=yes
hw.trackBall=no
image.sysdir.1=system-images/android-33/google_apis/x86_64/
runtime.network.latency=none
runtime.network.speed=full
showDeviceFrame=no
skin.dynamic=yes
skin.name=$SKIN
skin.path=$SKIN
tag.display=Google APIs
tag.id=google_apis
vm.heapSize=256
EOF

    echo "✅ AVD configured successfully!"
else
    echo "⚠️  Warning: AVD directory not found at $AVD_DIR"
fi

echo ""
echo "🎉 Android Emulator setup complete!"
echo ""
echo "📋 AVD Details:"
avdmanager list avd
echo ""
echo "🚀 To start the emulator:"
echo "   start-emulator.sh"
echo ""
echo "🔧 Manual emulator commands:"
echo "   emulator -avd $AVD_NAME                   # Start emulator"
echo "   emulator -avd $AVD_NAME -skin $SKIN       # Start with specific skin"
echo "   emulator -avd $AVD_NAME -no-skin          # Start without skin"
echo "   emulator -list-avds                       # List available AVDs"
