#!/bin/bash

# Copyright 2021 The Tint Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e # Fail on any error.

function show_cmds { set -x; }
function hide_cmds { { set +x; } 2>/dev/null; }
function task_begin {
    TASK_NAME="$@"
    SECONDS=0
}
function print_last_task_duration {
    if [ ! -z "${TASK_NAME}" ]; then
        echo "${TASK_NAME} completed in $(($SECONDS / 3600))h$((($SECONDS / 60) % 60))m$(($SECONDS % 60))s"
    fi
}
function status {
    echo ""
    echo ""
    print_last_task_duration
    echo ""
    echo "*****************************************************************"
    echo "* $@"
    echo "*****************************************************************"
    echo ""
    task_begin $@
}
function with_retry {
  local MAX_ATTEMPTS=5
  local RETRY_DELAY_SECS=5
  local ATTEMPT=1
  while true; do
    "$@" && break
    if [[ $ATTEMPT -ge $MAX_ATTEMPTS ]]; then
        echo "The command has failed after $ATTEMPT attempts."
        exit $?
    fi
    ((ATTEMPT++))
    echo "'$@' failed. Attempt ($ATTEMPT/$MAX_ATTEMPTS). Retrying..."
    sleep $RETRY_DELAY_SECS;
  done
}

ORIGINAL_SRC_DIR="$(pwd)"

. /bin/using.sh # Declare the bash `using` function for configuring toolchains.

using depot_tools
using go-1.14.4      # Speeds up ./tools/lint
using doxygen-1.8.18

status "Cloning to clean source directory"
# We do this so that the docker script can be tested in a local development
# checkout, without having the build litter the local checkout with artifacts
SRC_DIR=/tmp/tint-src
mkdir -p ${SRC_DIR}
cd ${SRC_DIR}
git clone ${ORIGINAL_SRC_DIR} .

status "Fetching dependencies"
cp standalone.gclient .gclient
with_retry gclient sync

status "Linting"
./tools/lint

status "Configuring build system"
if [ "$BUILD_SYSTEM" == "cmake" ]; then
    using cmake-3.17.2

    BUILD_DIR=/tmp/tint-build
    mkdir -p ${BUILD_DIR}

    COMMON_CMAKE_FLAGS=""
    COMMON_CMAKE_FLAGS+=" -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"

    if [ "$BUILD_TOOLCHAIN" == "clang" ]; then
        using clang-10.0.0
    elif [ "$BUILD_TOOLCHAIN" == "gcc" ]; then
        using gcc-9
    fi

    if [ "$BUILD_SANITIZER" == "asan" ]; then
        COMMON_CMAKE_FLAGS+=" -DTINT_ENABLE_ASAN=1"
    elif [ "$BUILD_SANITIZER" == "ubsan" ]; then
        COMMON_CMAKE_FLAGS+=" -DTINT_ENABLE_UBSAN=1"
        export UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
    fi

    cd ${BUILD_DIR}

    status "Building tint"
    show_cmds
        cmake ${SRC_DIR} ${CMAKE_FLAGS} ${COMMON_CMAKE_FLAGS}
        make --jobs=$(nproc)
    hide_cmds

    status "Running tint_unittests"
    show_cmds
        ./tint_unittests
    hide_cmds

    status "Testing test/test-all.sh"
    show_cmds
        ${SRC_DIR}/test/test-all.sh "${BUILD_DIR}/tint"
    hide_cmds

    status "Checking _other.cc files also build"
    show_cmds
        cmake ${SRC_DIR} ${CMAKE_FLAGS} ${COMMON_CMAKE_FLAGS} -DTINT_BUILD_AS_OTHER_OS=1
        make --jobs=$(nproc)
    hide_cmds
else
    status "Unsupported build system: $BUILD_SYSTEM"
    exit 1
fi

status "Done"
