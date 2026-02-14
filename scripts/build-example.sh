#!/bin/bash
set -e

cd "projects"

PROJECT_NAME=${1:-MmBasicForAndroid}
PACKAGE_NAME=${2:-com.sockpuppetstudios.mmb4a}

echo "🚀 Building SDL2 Example Project with Modern CMake: $PROJECT_NAME"

# Create the project
echo "📁 Creating SDL2 project..."
build-android-sdl2.sh "$PROJECT_NAME" "$PACKAGE_NAME"

cd "$PROJECT_NAME"

echo "🔧 Modernizing project to use CMake instead of ndk-build..."

# Create the modern source structure
mkdir -p app/src/main/cpp

# Copy the example main.cpp to the modern location
cp /workspace/main.cpp.example app/src/main/cpp/main.cpp

# Create a modern CMakeLists.txt that properly integrates SDL2
cat > app/src/main/cpp/CMakeLists.txt << 'EOF2'
cmake_minimum_required(VERSION 3.22.1)
project(SDL2Example)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find system libraries
find_library(LOG_LIBRARY log)
find_library(GLES_LIBRARY GLESv2)
find_library(ANDROID_LIBRARY android)

# Add SDL2 as a subdirectory (this builds SDL2 from source)
set(SDL2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../jni/SDL)
add_subdirectory(${SDL2_DIR} SDL2)

# Create our main library
add_library(main SHARED
    main.cpp
)

# Link libraries - SDL2 and SDL2main will be available after add_subdirectory
target_link_libraries(main
    SDL2
    SDL2main
    ${LOG_LIBRARY}
    ${GLES_LIBRARY}
    ${ANDROID_LIBRARY}
)

# Include SDL2 headers
target_include_directories(main PRIVATE
    ${SDL2_DIR}/include
)
EOF2

# Update build.gradle to use CMake instead of ndk-build
echo "📝 Updating build.gradle for CMake..."

# Backup the original build.gradle
cp app/build.gradle app/build.gradle.backup

# Create gradle.properties to enable AndroidX
cat > gradle.properties << 'EOF2'
android.useAndroidX=true
android.enableJetifier=true
org.gradle.jvmargs=-Xmx2048m -Dfile.encoding=UTF-8
android.enableR8.fullMode=true
android.nonTransitiveRClass=false
EOF2

# Create a simplified build.gradle with minimal dependencies
cat > app/build.gradle << 'EOF2'
plugins {
    id 'com.android.application'
}

android {
    namespace 'PACKAGE_PLACEHOLDER'
    compileSdk 33
    ndkVersion "25.2.9519653"

    defaultConfig {
        applicationId 'PACKAGE_PLACEHOLDER'
        minSdk 21
        targetSdk 33
        versionCode 1
        versionName "1.0"

        externalNativeBuild {
            cmake {
                cppFlags "-std=c++17"
                arguments "-DANDROID_STL=c++_shared"
            }
        }
        ndk {
            abiFilters 'arm64-v8a', 'armeabi-v7a', 'x86', 'x86_64'
        }
    }

    buildTypes {
        release {
            minifyEnabled false
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
        }
        debug {
            debuggable true
            jniDebuggable true
        }
    }

    externalNativeBuild {
        cmake {
            path file('src/main/cpp/CMakeLists.txt')
            version '3.22.1'
        }
    }

    compileOptions {
        sourceCompatibility JavaVersion.VERSION_17
        targetCompatibility JavaVersion.VERSION_17
    }

    sourceSets {
        main {
            jniLibs.srcDirs = ['libs']
        }
    }
}

dependencies {
    // Minimal dependencies - just what we need for SDL2
}
EOF2

# Replace the package placeholder with actual package name
sed -i "s/PACKAGE_PLACEHOLDER/$PACKAGE_NAME/g" app/build.gradle

# Update the Java activity to work with our setup
mkdir -p app/src/main/java/$(echo $PACKAGE_NAME | tr '.' '/')

# Create a simple activity that works with SDL2
cat > app/src/main/java/$(echo $PACKAGE_NAME | tr '.' '/')/MainActivity.java << 'EOF2'
package PACKAGE_PLACEHOLDER;

import org.libsdl.app.SDLActivity;

public class MainActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL2",
            "main"
        };
    }
}
EOF2

# Replace package placeholder in Java file
sed -i "s/PACKAGE_PLACEHOLDER/$PACKAGE_NAME/g" app/src/main/java/$(echo $PACKAGE_NAME | tr '.' '/')/MainActivity.java

# Update AndroidManifest.xml with simpler, compatible attributes
cat > app/src/main/AndroidManifest.xml << 'EOF2'
<manifest xmlns:android="http://schemas.android.com/apk/res/android">

    <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
    <uses-feature android:glEsVersion="0x00020000" android:required="true" />
    
    <application android:label="@string/app_name"
                 android:icon="@android:drawable/ic_menu_mylocation"
                 android:allowBackup="true"
                 android:theme="@android:style/Theme.Black.NoTitleBar.Fullscreen"
                 android:hardwareAccelerated="true">

        <activity android:name=".MainActivity"
                  android:label="@string/app_name"
                  android:alwaysRetainTaskState="true"
                  android:launchMode="singleInstance"
                  android:configChanges="layoutDirection|locale|orientation|uiMode|screenLayout|screenSize|smallestScreenSize|keyboardHidden"
                  android:exported="true"
                  android:screenOrientation="landscape">
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
EOF2

# Create strings.xml
mkdir -p app/src/main/res/values
cat > app/src/main/res/values/strings.xml << 'EOF2'
<resources>
    <string name="app_name">PROJECT_NAME_PLACEHOLDER</string>
</resources>
EOF2

# Replace project name placeholder
sed -i "s/PROJECT_NAME_PLACEHOLDER/$PROJECT_NAME/g" app/src/main/res/values/strings.xml

# Remove the old ndk-build files to avoid conflicts
rm -rf app/jni/src
rm -f app/jni/Android.mk app/jni/Application.mk 2>/dev/null || true

echo "🔨 Building the modernized project..."
./gradlew assembleDebug

if [ $? -eq 0 ]; then
    echo "✅ Build successful with modern CMake!"
    echo ""
    echo "📱 Your APK is ready at:"
    echo "   $(pwd)/app/build/outputs/apk/debug/app-debug.apk"
    echo ""
    echo "🚀 To install on a connected device:"
    echo "   adb install app/build/outputs/apk/debug/app-debug.apk"
    echo ""
    echo "📋 To view logs while running:"
    echo "   adb logcat -s SDL2App"
    echo ""
    echo "📁 Project structure:"
    echo "   app/src/main/cpp/main.cpp          - Your C++ source"
    echo "   app/src/main/cpp/CMakeLists.txt    - CMake build config"
    echo "   app/build.gradle                   - Modern Gradle config"
else
    echo "❌ Build failed. Check the error messages above."
    exit 1
fi
