# Dockerfile for Android Development with NDK and SDL2
FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Set environment variables
ENV ANDROID_HOME=/opt/android-sdk
ENV ANDROID_NDK_HOME=/opt/android-ndk
ENV SDL2_VERSION=2.28.5
ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
ENV PATH=${PATH}:${ANDROID_HOME}/cmdline-tools/latest/bin:${ANDROID_HOME}/platform-tools:${ANDROID_HOME}/emulator:${ANDROID_NDK_HOME}

# Install system dependencies including emulator requirements
RUN apt-get update && apt-get install -y \
    wget \
    unzip \
    openjdk-17-jdk \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    python3 \
    python3-pip \
    pkg-config \
    autoconf \
    automake \
    libtool \
    # Emulator dependencies
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libpulse0 \
    libxrandr2 \
    libxss1 \
    libxcursor1 \
    libxcomposite1 \
    libasound2 \
    libxi6 \
    libxtst6 \
    # Additional libraries for GUI support
    libgtk-3-0 \
    libdbus-glib-1-2 \
    libxt6 \
    libsm6 \
    libice6 \
    && rm -rf /var/lib/apt/lists/*

# Create directories
RUN mkdir -p ${ANDROID_HOME} ${ANDROID_NDK_HOME}

# Download and install Android Command Line Tools
RUN cd /tmp && \
    wget https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip && \
    unzip commandlinetools-linux-9477386_latest.zip && \
    mkdir -p ${ANDROID_HOME}/cmdline-tools && \
    mv cmdline-tools ${ANDROID_HOME}/cmdline-tools/latest && \
    rm commandlinetools-linux-9477386_latest.zip

# Accept Android SDK licenses and install components including emulator
RUN yes | sdkmanager --licenses && \
    sdkmanager "platform-tools" "platforms;android-33" "build-tools;33.0.2" \
    "emulator" "system-images;android-33;google_apis;x86_64"

# Download and install Android NDK
RUN cd /tmp && \
    wget https://dl.google.com/android/repository/android-ndk-r25c-linux.zip && \
    unzip android-ndk-r25c-linux.zip && \
    mv android-ndk-r25c/* ${ANDROID_NDK_HOME}/ && \
    rm -rf android-ndk-r25c android-ndk-r25c-linux.zip

# Download and build SDL2 for Android
RUN cd /tmp && \
    wget https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.tar.gz && \
    tar -xzf SDL2-${SDL2_VERSION}.tar.gz && \
    mkdir -p /opt/sdl2 && \
    mv SDL2-${SDL2_VERSION} /opt/sdl2/SDL2 && \
    rm SDL2-${SDL2_VERSION}.tar.gz

# Set up SDL2 Android project template
RUN cd /opt/sdl2/SDL2 && \
    cp -r android-project /opt/sdl2/android-template

# Install Gradle
RUN cd /opt && \
    wget https://services.gradle.org/distributions/gradle-8.5-bin.zip && \
    unzip gradle-8.5-bin.zip && \
    ln -s gradle-8.5 gradle && \
    rm gradle-8.5-bin.zip

# Set final environment - make sure emulator is in PATH
ENV PATH=${PATH}:/opt/gradle/bin:${ANDROID_HOME}/emulator

# Copy helper files
COPY scripts/ /usr/local/bin/
COPY examples/ /workspace/

# Make scripts executable
RUN chmod +x /usr/local/bin/*.sh

# Create a workspace directory
WORKDIR /workspace

# Create AVD directory
RUN mkdir -p /root/.android/avd

EXPOSE 5037 5554 5555

CMD ["/bin/bash"]
