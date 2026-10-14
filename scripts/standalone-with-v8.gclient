# Copy this file to <dawn clone dir>/.gclient to bootstrap gclient in a
# standalone checkout of Dawn that also compiles dawn_standalone_cts_runner.

solutions = [
    {
        "name": ".",
        "url": "https://dawn.googlesource.com/dawn",
        "deps_file": "DEPS",
        "managed": False,
        "custom_vars": {
            "checkout_v8": True,
            "dawn_node": True,
        }
    },
]
