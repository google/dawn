#!/bin/bash

# Copyright 2021 The Tint Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e # Fail on any error.

TEXT_GREEN="\033[0;32m"
TEXT_RED="\033[0;31m"
TEXT_DEFAULT="\033[0m"

TINT=$1

if [ ! -x "$TINT" ]; then
    echo "test-all.sh compiles with tint all the .wgsl files in the tint/test"
    echo "directory, for each of the SPIR-V, MSL, HLSL and WGSL backends."
    echo "Any errors are reported as test failures."
    echo ""
    echo "Usage: test-all.sh <path-to-tint-executable>"
    exit 1
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
NUM_PASS=0
NUM_FAIL=0

# check(WGSL_FILE, FORMAT)
function check() {
    WGSL_FILE=$1
    FORMAT=$2
    printf "%7s: " "${FORMAT}"
    set +e
    ${TINT} ${WGSL_FILE} --format ${FORMAT} -o /dev/null
    if [ $? -eq 0 ]; then
        echo -e "${TEXT_GREEN}PASS${TEXT_DEFAULT}"
        NUM_PASS=$((${NUM_PASS}+1))
    else
        echo -e "${TEXT_RED}FAIL${TEXT_DEFAULT}"
        NUM_FAIL=$((${NUM_FAIL}+1))
    fi
    set -e
}

for WGSL_FILE in ${SCRIPT_DIR}/*.wgsl
do
    echo
    echo "Testing $WGSL_FILE..."
    check "${WGSL_FILE}" wgsl
    check "${WGSL_FILE}" spirv
    check "${WGSL_FILE}" msl
    check "${WGSL_FILE}" hlsl
done

if [ ${NUM_FAIL} -ne 0 ]; then
    echo
    echo -e "${TEXT_RED}${NUM_FAIL} tests failed. ${TEXT_DEFAULT}${NUM_PASS} passed${TEXT_DEFAULT}"
    echo
    exit 1
else
    echo
    echo -e "${TEXT_GREEN}All ${NUM_PASS} tests pass${TEXT_DEFAULT}"
    echo
fi
