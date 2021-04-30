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

TEXT_YELLOW="\033[0;33m"
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
NUM_SKIP=0
NUM_FAIL=0

SKIPPED=""
SKIPPED+="msl:bug_tint_749.spvasm"  # TINT_UNIMPLEMENTED crbug.com/tint/726: module-scope private and workgroup variables not yet implemented
SKIPPED+="hlsl:bug_tint_749.spvasm" # Failed to generate: error: pointers not supported in HLSL

# should_skip(TEST_FILE, FORMAT)
function should_skip() {
    local TEST="$1-$2"
    if [[ "$TEST" == "bug_tint_749.spvasm-msl" ]]; then
        echo 1
        return
    fi
    echo 0
    return
}

# check(TEST_FILE, FORMAT)
function check() {
    local TEST_FILE=$1
    local FORMAT=$2
    SKIP=

    if [[ $SKIPPED == *"${FORMAT}:${TEST_FILE}"* ]]; then
        SKIP=1
    fi

    printf "%7s: " "${FORMAT}"
    if [[ -n "$SKIP" ]]; then
        echo -e "${TEXT_YELLOW}SKIPPED${TEXT_DEFAULT}"
        NUM_SKIP=$((${NUM_SKIP}+1))
        return
    fi
    set +e
    ${TINT} ${SCRIPT_DIR}/${TEST_FILE} --format ${FORMAT} -o /dev/null
    if [ $? -eq 0 ]; then
        echo -e "${TEXT_GREEN}PASS${TEXT_DEFAULT}"
        NUM_PASS=$((${NUM_PASS}+1))
    else
        echo -e "${TEXT_RED}FAIL${TEXT_DEFAULT}"
        NUM_FAIL=$((${NUM_FAIL}+1))
    fi
    set -e
}

for TEST_FILE in ${SCRIPT_DIR}/*.spvasm ${SCRIPT_DIR}/*.wgsl
do
    if [ -x realpath ]; then
      TEST_FILE=$(realpath --relative-to="$SCRIPT_DIR" "$TEST_FILE")
    else
      TEST_FILE=$(echo -n "$TEST_FILE"| sed -e "s'${SCRIPT_DIR}/*''")
    fi
    echo
    echo "Testing $TEST_FILE..."
    check "${TEST_FILE}" wgsl
    check "${TEST_FILE}" spirv
    check "${TEST_FILE}" msl
    check "${TEST_FILE}" hlsl
done

if [ ${NUM_FAIL} -ne 0 ]; then
    echo
    echo -e "${TEXT_RED}${NUM_FAIL} tests failed. ${TEXT_DEFAULT}${NUM_SKIP} skipped. ${NUM_PASS} passed.${TEXT_DEFAULT}"
    echo
    exit 1
else
    echo
    echo -e "${NUM_SKIP} tests skipped. ${TEXT_GREEN}${NUM_PASS} passed.${TEXT_DEFAULT}"
    echo
fi
