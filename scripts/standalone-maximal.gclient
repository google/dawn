# Copy this file to <dawn clone dir>/.gclient to bootstrap gclient in a
# standalone checkout of Dawn for building emdawnwebgpu using emsdk.
#
# Use this instead of `standalone.gclient` if you are a Dawn developer and
# would like to compile as much as possible on your local machine.

solutions = [
  { "name"        : ".",
    "url"         : "https://dawn.googlesource.com/dawn",
    "deps_file"   : "DEPS",
    "managed"     : False,
    "custom_vars" : {
      "checkout_clang_tidy": True,
      "checkout_clangd": True,
      "dawn_node" : True,
      "dawn_wasm" : True,
    }
  },
]
