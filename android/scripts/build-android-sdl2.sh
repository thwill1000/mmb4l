#!/bin/bash
set -e

PROJECT_NAME=${1:-MmBasicForAndroid}
PACKAGE_NAME=${2:-com.sockpuppetstudios.mmb4a}

echo "Creating SDL2 Android project: $PROJECT_NAME"

# Copy template
cp -r /opt/sdl2/android-template "$PROJECT_NAME"
cd "$PROJECT_NAME"

# Update project configuration
sed -i "s/org.libsdl.app/$PACKAGE_NAME/g" app/src/main/AndroidManifest.xml
sed -i "s/SDLActivity/${PROJECT_NAME}Activity/g" app/src/main/AndroidManifest.xml

# Create symbolic link to SDL2 source
ln -sf /opt/sdl2/SDL2 app/jni/SDL

echo "Project created successfully!"
echo "To build:"
echo "  cd $PROJECT_NAME"
echo "  ./gradlew assembleDebug"
