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

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"

function usage() {
    echo "test-all.sh is a simple wrapper around <tint>/tools/test-runner that"
    echo "injects the <tint>/tools directory as the second command line argument"
    echo
    echo "Usage of <tint>/tools/test-runner:"
    "${SCRIPT_DIR}/../tools/test-runner" --help
}

TINT="$1"

if [ -z "$TINT" ]; then
    echo "error: missing argument: location of the 'tint' executable"
    echo
    usage
    exit 1
fi
if [ ! -x "$TINT" ]; then
    echo "error: invalid argument: location of the 'tint' executable"
    echo
    usage
    exit 1
fi

"${SCRIPT_DIR}/../tools/test-runner" ${@:2} "${TINT}" "${SCRIPT_DIR}"
