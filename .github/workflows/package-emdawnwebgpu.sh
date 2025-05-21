#!/bin/bash
set -euo pipefail

# Version format: vYYYYMMDD.HHMMSS or vYYYYMMDD.HHMMSS-FORKOWNER.dawn.BRANCHNAME
VERSION_DATETIME=$(git show -s --date=format:'%Y%m%d.%H%M%S' --format=%cd)
VERSION_SUFFIX=${GITHUB_REPOSITORY/\//.}.${GITHUB_REF_NAME}
if [[ "$VERSION_SUFFIX" != "google.dawn.main" ]] ; then
    PKG_VERSION=v${VERSION_DATETIME}-${VERSION_SUFFIX}
else
    PKG_VERSION=v${VERSION_DATETIME}
fi
PKG_FILE=emdawnwebgpu_pkg-${PKG_VERSION}.zip

# Variables for documentation.
SHA=$(git rev-parse HEAD)
EMSDK_VERSION=$(python3 tools/activate-emsdk --get-emsdk-version)

# Initialize dependencies. We could use gclient for this, but then we still have to
# install gclient, and it takes a long time. We only need a few deps for emdawnwebgpu.
git submodule update --init --depth=1 third_party/abseil-cpp
git submodule update --init --depth=1 third_party/googletest
git submodule update --init --depth=1 third_party/emsdk
python3 tools/activate-emsdk

# Build the package (which is not affected by the build type), and build the
# link test in release mode (with Closure, which verifies the JS to some extent)
mkdir -p out/wasm
third_party/emsdk/upstream/emscripten/emcmake cmake -S=. -B=out/wasm -DCMAKE_BUILD_TYPE=Release
make -j4 -C out/wasm emdawnwebgpu_pkg emdawnwebgpu_link_test

# Also build the link test in debug mode.
mkdir -p out/wasm-debug
third_party/emsdk/upstream/emscripten/emcmake cmake -S=. -B=out/wasm-debug -DCMAKE_BUILD_TYPE=Debug
make -j4 -C out/wasm-debug emdawnwebgpu_link_test

# Create zip
cat << EOF > out/wasm/emdawnwebgpu_pkg/VERSION.txt
Dawn release ${PKG_VERSION} at revision <https://dawn.googlesource.com/dawn/+/${SHA}>.
Built/tested with emsdk release ${EMSDK_VERSION}.
EOF
(cd out/wasm && zip -9 -r ../../${PKG_FILE} emdawnwebgpu_pkg)

# Save version numbers for later steps
if [[ -n "${GITHUB_OUTPUT:-}" ]] ; then
    echo "PKG_VERSION=$PKG_VERSION" >> $GITHUB_OUTPUT
    echo "PKG_FILE=$PKG_FILE" >> $GITHUB_OUTPUT
fi
