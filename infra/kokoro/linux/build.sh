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

set -e # Fail on any error.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
ROOT_DIR="$( cd "${SCRIPT_DIR}/../../.." >/dev/null 2>&1 && pwd )"

# Inside the docker VM, we clone the project to a new directory.
# We do this so that the docker script can be tested in a local development
# checkout, without having the build litter the local checkout with artifacts.
# This directory is mapped to the host temporary directory.
# Kokoro uses a '/tmpfs' root, where as most linux enviroments just have '/tmp'
if [ -d "/tmpfs" ]; then
    TMP_DIR=/tmpfs
else
    TMP_DIR=/tmp
fi

echo "*****************************************************************"
echo "* build.sh"
echo "*"
echo "* df:"
df
echo "*****************************************************************"

# --privileged is required for some sanitizer builds, as they seem to require PTRACE privileges
docker run --rm -i \
  --privileged \
  --volume "${ROOT_DIR}:${ROOT_DIR}" \
  --volume "${TMP_DIR}/kokoro/dawn:/dawn" \
  --volume "${KOKORO_ARTIFACTS_DIR}:/mnt/artifacts" \
  --workdir "${ROOT_DIR}" \
  --env SRC_DIR="/dawn/src" \
  --env BUILD_DIR="/dawn/build" \
  --env BUILD_TYPE=$BUILD_TYPE \
  --env BUILD_SYSTEM=$BUILD_SYSTEM \
  --env BUILD_SANITIZER=$BUILD_SANITIZER \
  --env BUILD_TOOLCHAIN=$BUILD_TOOLCHAIN \
  --entrypoint "${SCRIPT_DIR}/docker.sh" \
  "gcr.io/shaderc-build/radial-build:latest"
