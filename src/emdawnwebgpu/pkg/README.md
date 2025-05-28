# Emdawnwebgpu

Emdawnwebgpu is Dawn's implementation of webgpu.h for Emscripten (on top of the
WebGPU JS API). It is a fork of Emscripten's original `USE_WEBGPU` bindings,
and while it is maintained in Dawn, it works in any browser supporting WebGPU
(modulo individual feature support).

The package includes all of the necessary files to use
`<webgpu/webgpu.h>` and the Dawn-style `<webgpu/webgpu_cpp.h>` with Emscripten.

Find new versions of this package at <https://github.com/google/dawn/releases>.

If you find any issue with this release, please verify it in the latest release,
and then report it at <https://crbug.com/new?component=1570785&noWizard=True>.

## How to use this package

First, start with either:

- A "remote" port file `emdawnwebgpu-v*.remoteport.py` (requires Emscripten 4.0.10+).
- An `emdawnwebgpu_pkg` containing a local port file `emdawnwebgpu.port.py`.
  (Either from a pre-built zip release, or from a Dawn build output directory.)

## How to use this package (local or remote)

Pass the following flag to `emcc`, during both compile and link, to set the
include paths and link the implementation:

    --use-port=path/to/emdawnwebgpu_port_or_remoteport_file.py

If (and only if) using Emscripten before 4.0.7, also pass this flag during link:

    --closure-args=--externs=path/to/emdawnwebgpu_pkg/webgpu/src/webgpu-externs.js

## Port options

Options can be set by appending `:key1=value:key2=value` to `--use-port`.
For information about port options, run:

    emcc --use-port=emdawnwebgpu:help
    emcc --use-port=path/to/emdawnwebgpu.port.py:help
    emcc --use-port=path/to/emdawnwebgpu-*.remoteport.py:help

### C++ bindings

By default, C++ bindings are provided in the include path. Note that unlike
`webgpu.h`, these are not intended to be fully stable. If you don't want these
for any reason (you have custom bindings, you're using a pinned snapshot of
`webgpu_cpp.h`, etc.), you can set the option `cpp_bindings=false`:

    --use-port=emdawnwebgpu:cpp_bindings=false
    --use-port=path/to/emdawnwebgpu.port.py:cpp_bindings=false
    --use-port=path/to/emdawnwebgpu-*.remoteport.py:cpp_bindings=false

## Embuilder

If your build process needs a separate step to build the port before linking,
use Emscripten's `embuilder`.

Under `embuilder`, some options cannot be set automatically, so they must be
set manually. See `OPTIONS` in `emdawnwebgpu.port.py` for details.
