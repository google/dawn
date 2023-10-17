#!/bin/bash

# Copyright 2021 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This is the bash script invoked inside a docker container.
# The script expects that the CWD points to a clean checkout of Dawn.
# As `gclient sync` will litter the dawn checkout with fetched tools and
# projects, this script will first clone the pristine dawn checkout to
# ${SRC_DIR}. This allows developers to locally run this script without having
# to worry about their local dawn copy being touched.
#
# This script expects the following environment variables to be set on entry:
#
# SRC_DIR         - Path to where the local Dawn copy will be made. See above.
# BUILD_DIR       - Path to where Dawn will be built.
# BUILD_TYPE      - Either: 'Debug' or 'Release'
# BUILD_SYSTEM    - Must be 'cmake'
# BUILD_SANITIZER - Either: '', 'asan', or 'ubstan'
# BUILD_TOOLCHAIN - Either: 'clang' or 'gcc'

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
    echo "*"
    echo "* df:"
    df
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

CLONE_SRC_DIR="$(pwd)"

. /bin/using.sh # Declare the bash `using` function for configuring toolchains.

using depot_tools
using go-1.18
using doxygen-1.9.5

status "Creating source directory '${SRC_DIR}' and build directory '${BUILD_DIR}'"
mkdir -p ${SRC_DIR}
mkdir -p ${BUILD_DIR}

status "Cloning to source directory '${SRC_DIR}'"
cd ${SRC_DIR}
git clone ${CLONE_SRC_DIR} .

status "Checking for CRLF"
./tools/check-no-crlf

status "Fetching dependencies"
cp scripts/standalone-with-node.gclient .gclient
with_retry gclient sync

status "Linting"
./tools/lint

if [ "$BUILD_SYSTEM" == "cmake" ]; then
    # TODO(crbug.com/dawn/1358): Remove the need to depend on GLFW.
    status "Installing GLFW system dependencies [crbug.com/dawn/1358]"
    sudo apt update
    sudo apt -y install \
        libgl-dev \
        libx11-dev \
        libx11-xcb-dev \
        libxcursor-dev \
        libxext-dev \
        libxi-dev \
        libxinerama-dev \
        libxrandr-dev
fi

status "Configuring build system"
if [ "$BUILD_SYSTEM" == "cmake" ]; then
    using cmake-3.17.2

    COMMON_CMAKE_FLAGS=""
    COMMON_CMAKE_FLAGS+=" -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
    COMMON_CMAKE_FLAGS+=" -DTINT_DOCS_WARN_AS_ERROR=1"
    COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_BENCHMARKS=1"
    COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_SPV_READER=1"
    COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_WGSL_READER=1"
    COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_GLSL_WRITER=1"
    COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_HLSL_WRITER=1"
    COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_MSL_WRITER=1"
    COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_SPV_WRITER=1"
    COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_WGSL_WRITER=1"
    COMMON_CMAKE_FLAGS+=" -DTINT_RANDOMIZE_HASHES=1"

    if [ "$BUILD_TOOLCHAIN" == "clang" ]; then
        using clang-13.0.1
        COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_FUZZERS=1"
        COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_SPIRV_TOOLS_FUZZER=1"
        COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_AST_FUZZER=1"
        COMMON_CMAKE_FLAGS+=" -DTINT_BUILD_REGEX_FUZZER=1"
    elif [ "$BUILD_TOOLCHAIN" == "gcc" ]; then
        using gcc-13
    fi

    if [ "$BUILD_SANITIZER" == "asan" ]; then
        COMMON_CMAKE_FLAGS+=" -DDAWN_ENABLE_ASAN=1"
    elif [ "$BUILD_SANITIZER" == "ubsan" ]; then
        COMMON_CMAKE_FLAGS+=" -DDAWN_ENABLE_UBSAN=1"
        export UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
    fi

    status "Running go tool unittests"
    show_cmds
        go test ./...
    hide_cmds

    cd ${BUILD_DIR}

    status "Running Doxygen"
    echo "NOTE: This will fail on first warning. Run with -DTINT_DOCS_WARN_AS_ERROR=OFF to see all warnings".
    echo ""
    show_cmds
        # NOTE: If we upgrade Doxygen to a more recent version, we can set DOXYGEN_WARN_AS_ERROR to
        # "FAIL_ON_WARNINGS" instead of "YES" in our CMakeLists.txt so see all warnings, and then
        # fail. See https://www.doxygen.nl/manual/config.html#cfg_warn_as_error
        cmake ${SRC_DIR} ${CMAKE_FLAGS} ${COMMON_CMAKE_FLAGS}
        cmake --build . --target tint-docs
    hide_cmds

    status "Building dawn in '${BUILD_DIR}'"
    show_cmds
        cmake ${SRC_DIR} ${CMAKE_FLAGS} ${COMMON_CMAKE_FLAGS}
        cmake --build . -- --jobs=$(nproc)
    hide_cmds

    status "Re-building dawn in '${BUILD_DIR}' with dawn/node enabled"
    show_cmds
        cmake ${SRC_DIR} ${CMAKE_FLAGS} ${COMMON_CMAKE_FLAGS} -DDAWN_BUILD_NODE_BINDINGS=1 -DDAWN_ENABLE_PIC=1 -DDAWN_USE_X11=OFF
        cmake --build . -- --jobs=$(nproc)
    hide_cmds

    status "Running tint_unittests"
    show_cmds
        ./tint_unittests
    hide_cmds

    if [ -f ./tint_ast_fuzzer_unittests ]; then
        status "Running tint_ast_fuzzer_unittests"
        show_cmds
            ./tint_ast_fuzzer_unittests
        hide_cmds
    fi

    if [ -f ./tint_regex_fuzzer_unittests ]; then
        status "Running tint_regex_fuzzer_unittests"
        show_cmds
            ./tint_regex_fuzzer_unittests
        hide_cmds
    fi

    status "Testing test/tint/test-all.sh"
    show_cmds
        ${SRC_DIR}/test/tint/test-all.sh "${BUILD_DIR}/tint" --verbose
    hide_cmds

    status "Testing test/tint/test-all.sh for SPIR-V IR backend"
    show_cmds
        ${SRC_DIR}/test/tint/test-all.sh "${BUILD_DIR}/tint" --verbose --format spvasm --use-ir
    hide_cmds

    status "Checking _other.cc files also build"
    show_cmds
        cmake ${SRC_DIR} ${CMAKE_FLAGS} ${COMMON_CMAKE_FLAGS} -DTINT_BUILD_AS_OTHER_OS=ON
        cmake --build . -- --jobs=$(nproc)
        cmake ${SRC_DIR} ${CMAKE_FLAGS} ${COMMON_CMAKE_FLAGS} -DTINT_BUILD_AS_OTHER_OS=OFF
    hide_cmds

    status "Checking disabling all readers and writers also builds"
    show_cmds
        cmake ${SRC_DIR} ${CMAKE_FLAGS} ${COMMON_CMAKE_FLAGS} \
            -DTINT_BUILD_SPV_READER=OFF \
            -DTINT_BUILD_SPV_WRITER=OFF \
            -DTINT_BUILD_WGSL_READER=OFF \
            -DTINT_BUILD_WGSL_WRITER=OFF \
            -DTINT_BUILD_MSL_WRITER=OFF \
            -DTINT_BUILD_HLSL_WRITER=OFF \
            -DTINT_BUILD_GLSL_WRITER=OFF \
            -DTINT_BUILD_GLSL_VALIDATOR=OFF \
            -DTINT_BUILD_BENCHMARKS=OFF
        cmake --build . -- tint --jobs=$(nproc)
        cmake ${SRC_DIR} ${CMAKE_FLAGS} ${COMMON_CMAKE_FLAGS} \
            -DTINT_BUILD_SPV_READER=ON \
            -DTINT_BUILD_SPV_WRITER=ON \
            -DTINT_BUILD_WGSL_READER=ON \
            -DTINT_BUILD_WGSL_WRITER=ON \
            -DTINT_BUILD_MSL_WRITER=ON \
            -DTINT_BUILD_HLSL_WRITER=ON \
            -DTINT_BUILD_GLSL_WRITER=ON \
            -DTINT_BUILD_GLSL_VALIDATOR=ON \
            -DTINT_BUILD_BENCHMARKS=ON
    hide_cmds
else
    status "Unsupported build system: $BUILD_SYSTEM"
    exit 1
fi

status "Done"
