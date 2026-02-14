#!/bin/bash

# Build and run Android NDK + SDL2 development container

set -e

echo "🔨 Building Android NDK + SDL2 Docker image..."

# Build the Docker image
docker-compose build

echo "✅ Build complete!"

echo "🚀 Starting the development container..."

# Start the container
docker-compose up -d

echo "📱 Container is running! You can now:"
echo ""
echo "1. Enter the container:"
echo "   docker-compose exec android-dev bash"
echo ""
echo "2. Create a new SDL2 project:"
echo "   docker-compose exec android-dev build-android-sdl2.sh MyGame com.example.mygame"
echo ""
echo "3. Build your project:"
echo "   docker-compose exec android-dev bash -c 'cd MyGame && ./gradlew assembleDebug'"
echo ""
echo "4. Stop the container when done:"
echo "   docker-compose down"
echo ""

# Optional: Enter the container immediately
read -p "Do you want to enter the container now? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    docker-compose exec android-dev bash
fi
