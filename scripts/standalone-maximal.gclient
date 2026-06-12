# Copy this file to <dawn clone dir>/.gclient to bootstrap gclient in a
# standalone checkout of Dawn for building emdawnwebgpu using emsdk.
#
# Use this instead of `standalone.gclient` if you are a Dawn developer and
# would like to compile as much as possible on your local machine.

import platform

solutions = [
    {
        "name": ".",
        "url": "https://dawn.googlesource.com/dawn",
        "deps_file": "DEPS",
        "managed": False,
        "custom_vars": {
            #"download_remoteexec_cfg": True,  # for Siso
            "checkout_clang_tidy": True,
            "checkout_clangd": True,
            "dawn_node": True,
            "dawn_wasm": True,
        }
    },
]

# Enable all additional targets that are usable on this host OS.
target_os = []
if platform.system() == "Darwin":
    target_os += ['win']
elif platform.system() == "Linux":
    target_os += ['win', 'mac']
