#!/bin/bash
set -e

if [ -z "$ANDROID_HOME" ]; then
  echo "ANDROID_HOME is not set"
  exit 1
fi

NDK_PATH=$(ls -d "$ANDROID_HOME"/ndk/* | sort -V | tail -n 1)
if [ ! -d "$NDK_PATH" ]; then
  echo "NDK not found"
  exit 1
fi

BUILD_DIR="$PWD/build"
mkdir -p "$BUILD_DIR"

"$NDK_PATH/ndk-build" \
  NDK_PROJECT_PATH=. \
  APP_BUILD_SCRIPT=Android.mk \
  NDK_APPLICATION_MK=Application.mk \
  NDK_OUT="$BUILD_DIR"

echo "Build complete"
