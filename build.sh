#!/bin/bash

# Exit on any failure.
set -e

source /etc/os-release

IMAGE_BASE_NAME="$USER-sdl2"

function docker_parse_image_type() {
  case "$1" in
    armv6l)
      DOCKER_PLATFORM="linux/arm/v6"
      DOCKER_EXTRA="-e QEMU_CPU=arm1176"
      ;;
    arm64|aarch64)
      DOCKER_PLATFORM="linux/arm64"
      DOCKER_EXTRA=""
      ;;
    amd64|x86_64)
      DOCKER_PLATFORM="linux/amd64"
      DOCKER_EXTRA=""
      ;;
    *)
      echo "Unknown docker image type: $1"
      exit 1
      ;;
  esac
  DOCKER_IMAGE="$IMAGE_BASE_NAME/$DOCKER_PLATFORM"
}

function docker_create_image() {
  if ! docker buildx ls | grep -q 'multi-platform-builder'; then
    docker buildx create --use --platform=linux/arm/v6,linux/arm64,linux/amd64 --name multi-platform-builder
  fi
  docker buildx inspect --bootstrap multi-platform-builder
  docker run --rm --privileged multiarch/qemu-user-static --reset -p yes -c yes
  docker buildx build --load -t $IMAGE_BASE_NAME/$DOCKER_PLATFORM --platform $DOCKER_PLATFORM --build-arg USERNAME=$USER ./docker
}

function docker_run_command() {
  if [ "$DOCKER_PLATFORM" == "linux/arm/v6" ]; then
    docker run -it --rm -e DISPLAY=$DISPLAY $DOCKER_EXTRA --net=host --rm -v /home/$USER:/home/$USER $DOCKER_IMAGE $1
  else
    docker run -it --rm -e DISPLAY=$DISPLAY $DOCKER_EXTRA --platform $DOCKER_PLATFORM --net=host --rm -v /home/$USER:/home/$USER $DOCKER_IMAGE $1
  fi
}

function create_distribution() {
  cd $BUILD_DIR

  # Extract version from the executable.
  RAW_VERSION_STRING=`strings mmbasic | grep "@(#)"`
  RAW_VERSION_ARRAY=($RAW_VERSION_STRING) # splits on spaces
  MAJOR=`echo ${RAW_VERSION_ARRAY[2]:1} | cut -d '.' -f 1`
  MINOR=`echo ${RAW_VERSION_ARRAY[2]} | cut -d '.' -f 2`
  MICRO=`echo ${RAW_VERSION_ARRAY[2]} | cut -d '.' -f 3`
  if (( MICRO > 300 )); then
    MICRO="."$((MICRO-300))
  elif (( MICRO > 200 )); then
    MICRO="-rc."$((MICRO-200))
  elif (( MICRO > 100 )); then
    MICRO="-beta."$((MICRO-100))
  else
    MICRO="-alpha."${MICRO}
  fi

  VERSION=${MAJOR}"."${MINOR}${MICRO}
  GLIBC=`$BASE_DIR/tools/glibc-check.sh -m ./mmbasic`
  DISTRIB_NAME=mmb4l-${VERSION}-${ARCH}-glibc-${GLIBC}

  \rm -Rf $DISTRIB_NAME
  mkdir -p $DISTRIB_NAME
  cp $BUILD_DIR/mmbasic $DISTRIB_NAME
  cp $BASE_DIR/resources/mmbasic.nanorc $DISTRIB_NAME
  cp $BASE_DIR/resources/mmbasic.syntax.nanorc $DISTRIB_NAME
  cp $BASE_DIR/ChangeLog $DISTRIB_NAME
  cp $BASE_DIR/LICENSE* $DISTRIB_NAME
  cp $BASE_DIR/README.md $DISTRIB_NAME

  tar -czvf $DISTRIB_NAME.tar.gz $DISTRIB_NAME
  mkdir -p $BASE_DIR/dist
  cp $DISTRIB_NAME.tar.gz $BASE_DIR/dist
}

source /etc/os-release

POSITIONAL_ARGS=()
BUILD_TYPE="release"
DOCKER_PLATFORM=""
CLEAN=false
ACTION="make"

while [[ $# -gt 0 ]]; do
  case $1 in
    -c|--clean)
      CLEAN=true
      shift # past argument
      ;;
    -m|--make)
      shift # past argument
      if [ -n "$1" ]; then
        docker_parse_image_type $1
        shift # past value
      fi
      ;;
    -i|--create-image)
      ACTION="create-image"
      shift # past argument
      docker_parse_image_type $1
      shift # past value
      ;;
    -r|--run)
      ACTION="run"
      shift # past argument
      if [ -n "$1" ]; then
        docker_parse_image_type $1
        shift # past value
      fi
      ;;
    -b|--start-bash)
      ACTION="start-bash"
      shift # past argument
      docker_parse_image_type $1
      shift # past value
      ;;
    -t|--type)
      BUILD_TYPE="$2"
      shift # past argument
      shift # past value
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      POSITIONAL_ARGS+=("$1") # save positional arg
      shift # past argument
      ;;
  esac
done

set -- "${POSITIONAL_ARGS[@]}" # restore positional parameters

# Validate build type.
case "$BUILD_TYPE" in
  release|debug|coverage)
    # Do nothing, this is valid.
    ;;
  *)
    echo "Unknown build type: $BUILD_TYPE"
    exit 1
    ;;
esac

BASE_DIR=`realpath $(dirname "$0")`

if [ "$DOCKER_PLATFORM" != "" ]; then
  case "$ACTION" in
    create-image)
      docker_create_image
      exit 0
      ;;
    make)
      docker_run_command "$BASE_DIR/build.sh --type $BUILD_TYPE --make"
      exit 0
      ;;
    run)
      docker_run_command "$BASE_DIR/build.sh --type $BUILD_TYPE --run"
      exit 0
      ;;
    start-bash)
      DOCKER_EXTRA="$DOCKER_EXTRA -w $BASE_DIR"
      docker_run_command "/bin/bash"
      exit 0
      ;;
    *)
      echo "Unknown action: $ACTION"
      exit 1
      ;;
  esac
fi

ARCH=`uname -m`
GCC_VERSION_FULL=`gcc --version | head -n 1`
GCC_VERSION=`echo ${GCC_VERSION_FULL##* }`
BUILD_DIR="${BASE_DIR}/build/build-${BUILD_TYPE}-${ARCH}-${ID}-${VERSION_ID}-gcc-${GCC_VERSION}"

# Run 'mmbasic' executable.
if [ "$ACTION" == "run" ]; then
  echo $BUILD_DIR
  cd $BUILD_DIR && ./mmbasic
  exit 0
fi

if [ "$ACTION" != "make" ]; then
  echo "Unknown action: $ACTION"
  exit 1
fi

# Delete 'mmbasic' executable or entire build directory if requested.
if $CLEAN; then
  \rm -Rf $BUILD_DIR
fi
rm -f $BUILD_DIR/mmbasic
mkdir -p $BUILD_DIR

cd $BUILD_DIR

# Configure build.
case "$BUILD_TYPE" in
  release)
    cmake -DCMAKE_BUILD_TYPE=Release $BASE_DIR
    ;;
  debug)
    cmake -DCMAKE_BUILD_TYPE=Debug $BASE_DIR
    ;;
  coverage)
    cmake -DMMB4L_COVERAGE=1 $BASE_DIR
    ;;
  *)
    echo "Unknown build type: $BUILD_TYPE"
    exit 1
    ;;
esac

# Compile.
make -j4

# Test.
ctest

# Create distribution .tar.gz.
create_distribution
