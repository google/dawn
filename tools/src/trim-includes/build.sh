#!/bin/bash

set -e # Fail on any error.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
ROOT_DIR="$( cd "${SCRIPT_DIR}/../.." >/dev/null 2>&1 && pwd )"

cd $ROOT_DIR
autoninja -C out/Debug
cd $ROOT_DIR/build
ninja
