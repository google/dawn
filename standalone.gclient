# Copy this file to <tint clone dir>/.gclient to bootstrap gclient in a
# standalone checkout of Tint.

solutions = [
  { "name"        : ".",
    "url"         : "https://dawn.googlesource.com/tint",
    "deps_file"   : "DEPS",
    "managed"     : False,
    "custom_deps": {
    },
  },
]
