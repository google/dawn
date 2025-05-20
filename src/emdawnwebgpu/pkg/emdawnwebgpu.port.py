# Copyright 2025 The Dawn & Tint Authors
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

# Emscripten port file for webgpu and webgpu_cpp. See README for how to use.
#
# Style note: Variables starting with _ are local. The rest are part of the
# Python module interface for Emscripten ports.

import os
import zlib
from typing import Union, Dict, Optional

LICENSE = 'mixed licenses, see license files'

# User options, e.g. --use-port=path/to/emdawnwebgpu.port.py:cpp_bindings=false
OPTIONS = {
    'cpp_bindings':
    "Add the include path for Dawn-like <webgpu/webgpu_cpp.h> C++ bindings. Default: true.",
    'opt_level':
    "Optimization (-O) level for the bindings' Wasm layer. Default: choose based on -sASSERTIONS.",
}
_VALID_OPTION_VALUES = {
    'cpp_bindings': ['true', 'false'],
    'opt_level': ['auto', '0', '2'],
}
_opts: Dict[str, Union[Optional[str], bool]] = {
    'cpp_bindings': True,
    'opt_level': 'auto',
}


def _walk(path):
    for (dirpath, dirnames, filenames) in os.walk(path):
        for filename in filenames:
            yield os.path.join(dirpath, filename)


_pkg_dir = os.path.dirname(os.path.realpath(__file__))
_c_include_dir = os.path.join(_pkg_dir, 'webgpu', 'include')
_cpp_include_dir = os.path.join(_pkg_dir, 'webgpu_cpp', 'include')
_src_dir = os.path.join(_pkg_dir, 'webgpu', 'src')
_srcs = [
    os.path.join(_src_dir, 'webgpu.cpp'),
]

# Check for a generated file that would only be there in the built package
if not os.path.isfile(os.path.join(_c_include_dir, 'webgpu', 'webgpu.h')):
    raise Exception(
        "emdawnwebgpu.port.py may only be used from a built emdawnwebgpu_pkg, "
        "not from Dawn's source tree. You can use pre-built packages from "
        "https://github.com/google/dawn/releases or build it locally.")

# Collect a list of all files that affect the compiled port so that we know
# when to recompile it. (Normally Emscripten handles this, but not here because
# of the way that we "misuse" the ports system.)
_files_affecting_port_build = sorted([
    __file__,
    *_srcs,
    *_walk(_c_include_dir),
])

# Hooks that affect compiler invocations


def _check_option(option, value, error_handler):
    if value not in _VALID_OPTION_VALUES[option]:
        error_handler(
            f'[{option}] can be {list(_VALID_OPTION_VALUES[option])}, got [{value}]'
        )
    if isinstance(_opts[option], bool):
        value = value == 'true'
    return value


def handle_options(options, error_handler):
    for option, value in options.items():
        value = value.lower()
        _opts[option] = _check_option(option, value, error_handler)


def process_args(ports):
    # It's important that these take precedent over Emscripten's builtin
    # system/include/, which also currently has webgpu headers.
    args = ['-isystem', _c_include_dir]
    if _opts['cpp_bindings']:
        args += ['-isystem', _cpp_include_dir]
    return args


# Hooks that affect linker invocations


# If not explicitly set, use -O0 when linking with -sASSERTIONS builds, and -O2
# otherwise. (ASSERTIONS implicitly affects library_webgpu.js, so it makes sense
# for it to control this too - debuggability is most useful with assertions.)
# Emscripten automatically handles necessary compile flags (LTO, PIC, wasm64).
def _compute_opt_level_flag(settings):
    value = _opts['opt_level']
    if value == 'auto':
        value = '0' if settings.ASSERTIONS else '2'
    return f'-O{value}'


# Compute the library compile flags, either `-O0` or `-O2 -DNDEBUG` (see above).
# NDEBUG affects <assert.h> and is used in `webgpu.cpp`.
def _compute_library_compile_flags(settings):
    opt_level_flag = _compute_opt_level_flag(settings)
    flags = [opt_level_flag]
    # Set NDEBUG if -O0 (similar to emcc's defaulting of -sASSERTIONS from -O).
    if opt_level_flag != '-O0':
        flags.append('-DNDEBUG')
    return flags


# Create a unique lib name for this version of the port and compile flags.
def _get_lib_name(settings):
    # Compute a hash from all of the inputs to ports.build_port() so that
    # Emscripten knows when it needs to recompile.
    hash_value = 0

    def add(x):
        nonlocal hash_value
        hash_value = zlib.adler32(x, hash_value)

    for filename in _files_affecting_port_build:
        add(open(filename, 'rb').read())

    opt_level_flag = _compute_opt_level_flag(settings)
    return f'lib_emdawnwebgpu-{hash_value:08x}{opt_level_flag}.a'


def linker_setup(ports, settings):
    if settings.USE_WEBGPU:
        raise Exception('emdawnwebgpu may not be used with -sUSE_WEBGPU=1')

    # These will be processed in order; library_webgpu.js must come after
    # the generated files, because it depends on them.
    settings.JS_LIBRARIES += [
        os.path.join(_src_dir, 'library_webgpu_enum_tables.js'),
        os.path.join(_src_dir, 'library_webgpu_generated_struct_info.js'),
        os.path.join(_src_dir, 'library_webgpu_generated_sig_info.js'),
        os.path.join(_src_dir, 'library_webgpu.js'),
    ]
    if 'CLOSURE_ARGS' in settings.keys():
        # This works in Emscripten >4.0.7. In <=4.0.7, the user has to pass it.
        settings.CLOSURE_ARGS += [
            '--externs=' + os.path.join(_src_dir, 'webgpu-externs.js'),
        ]


def get(ports, settings, shared):
    if settings.allowed_settings:
        # We are in the compile phase. The lib_emdawnwebgpu-*.a library file
        # isn't needed until linking.
        return []

    def create(final):
        # Note we don't use ports.install_header; instead we directly add the
        # include path via process_args(). The only thing we cache is the
        # compiled webgpu.cpp (which also includes webgpu/webgpu.h).
        includes = [_c_include_dir]
        # Always use -g. The linker can remove debug symbols in release builds.
        flags = ['-g', '-std=c++17', '-fno-exceptions']
        flags += _compute_library_compile_flags(settings)

        # IMPORTANT: Keep `_files_affecting_port_build` in sync with this.
        ports.build_port(_src_dir,
                         final,
                         'emdawnwebgpu',
                         includes=includes,
                         flags=flags,
                         srcs=_srcs)

    lib_name = _get_lib_name(settings)
    return [shared.cache.get_lib(lib_name, create, what='port')]


def clear(ports, settings, shared):
    shared.cache.erase_lib(_get_lib_name(settings))
