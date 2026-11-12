use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

gclient_gn_args = [
  'android_ndk_version',
  'build_with_chromium',
  'checkout_v8',
  'dawn_node',
  'dawn_wasm',
  'generate_location_tags',
]

git_dependencies = 'SYNC'

vars = {
  'boringssl_git': 'https://boringssl.googlesource.com',
  'chromium_git': 'https://chromium.googlesource.com',
  'dawn_git': 'https://dawn.googlesource.com',
  'github_git': 'https://github.com',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'dawn_standalone': True,
  'dawn_node': False, # Also fetches dependencies required for building NodeJS bindings.
  'dawn_wasm': False, # Also fetches dependencies required for building WebAssembly.
  'dawn_cmake_version': 'version:2@3.23.3',
  'dawn_cmake_win32_sha1': 'b106d66bcdc8a71ea2cdf5446091327bfdb1bcd7',
  'dawn_gn_version': 'git_revision:4df39f484f1f49f5c7f6264adc88d27811cb7b00',
  # ninja CIPD package version.
  # https://chrome-infra-packages.appspot.com/p/infra/3pp/tools/ninja
  'dawn_ninja_version': 'version:3@1.12.1.chromium.4',
  'dawn_go_version': 'version:3@1.26.6',
  'dawn_node_version': 'version:2@20.11.0',
  'dawn_agility_sdk_version': 'version:2@1.722.0-preview',
  'dawn_direct3d_warp_version': 'version:3@1.0.21.chromium.1',
  'dawn_bazelisk_version': 'version:3@1.29.0',
  'dawn_llvm-dev_version': 'version:3@22.1.0',
  'dawn_zstd_version': 'Lf-seQJdussz81cZXnez5xIxEdkejDRxKumQpHbHMOUC',

  # GN variable required by //testing that will be output in the gclient_args.gni
  'generate_location_tags': False,

  # Fetch clang-tidy into the same bin/ directory as our clang binary,
  # and Chromium's tools/build for tricium_clang_tidy_script.py.
  'checkout_clang_tidy': False,

  # Fetch clangd into the same bin/ directory as our clang binary.
  'checkout_clangd': False,

  # Fetch configuration files required for the 'use_remoteexec' gn arg
  'download_remoteexec_cfg': False,
  # RBE instance to use for running remote builds
  'rbe_instance': 'projects/rbe-chrome-untrusted/instances/default_instance',
  # RBE project to download rewrapper config files for. Only needed if
  # different from the project used in 'rbe_instance'
  'rewrapper_cfg_project': '',
  # siso CIPD package version.
  'siso_version': 'git_revision:59d84e3b054313652b74928f5a9c550f4a951723',

  # CPython 3 CIPD package version for Siso hermetic toolchain.
  'cpython3_version': 'version:3@3.11.9.chromium.38',

  # 'magic' text to tell depot_tools that git submodules should be accepted
  # but parity with DEPS file is expected.
  'SUBMODULE_MIGRATION': 'True',

  'fetch_cmake': False,
  'fetch_bazel': False,

  # condition to allowlist deps to be synced in Cider. Allowlisting is needed
  # because not all deps are compatible with Cider. Once we migrate everything
  # to be compatible we can get rid of this allowlisting mecahnism and remove
  # this condition. Tracking bug for removing this condition: b/349365433
  'non_git_source': 'True',

  # Set to True by Chromium if syncing from a Chromium checkout.
  'build_with_chromium': False,

  # Version of Chromium the DEPS entries synced by scripts/roll_chromium_deps.py
  # were last synced to.
  'chromium_revision': '9d94f63824e89f199c1e94220d71ffd8afda6c25',
  # We never want to actually checkout Chromium, but we need a fake DEPS entry
  # in order for the Chromium -> Dawn DEPS autoroller to work.
  'checkout_placeholder_chromium': False,

  # Checkout mesa 3D graphics library and related dependencies
  # Not used by Dawn/Tint directly, but is used as part of extended fuzzing stack.
  'checkout_mesa': False,

  # Checkout //tools/code_coverage from Chromium and fetches the
  # prebuilt versions of llvm-cov and llvm-profdata.
  'checkout_clang_coverage_tools': False,

  # Checkout LiteRT-LM and its data dependencies.
  # Not actually depended on by Dawn/Tint, only used to run benchmark tests on them.
  'checkout_litert_lm': False,

  # Checkout V8 and its dependencies for standalone builds (e.g. CTS runner).
  'checkout_v8': False,

  # Dependency versions for Android builds.
  # These should be all rolled at once to match the values in Chromium.
  'android_ndk_version': Str('2@30.0.16248370'),
  'android_sdk_build-tools_version': 'version_37.0.0',
  'android_sdk_emulator_version': '9lGp8nTUCRRWGMnI_96HcKfzjnxEJKUcfvfwmA3wXNkC',
  'android_sdk_platform-tools_version': 'qTD9QdBlBf3dyHsN1lJ0RH6AhHxR42Hmg2Ih-Vj4zIEC',
  'android_sdk_platforms_version': 'AAswZ5rZBFDhMaH4QbKPmVtP4vnjiNUW_zPhdWvNe6wC',
}

deps = {
  'buildtools': {
    'url': '{chromium_git}/chromium/src/buildtools@a9c58ac990557a29176bbaeef7577b196c38a950',
    'condition': 'dawn_standalone',
  },
  'third_party/clang-format/script': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@a5bc718d465e842f6e637749a718748284b5ef80',
    'condition': 'dawn_standalone',
  },
  'buildtools/linux64': {
    'packages': [{
      'package': 'gn/gn/linux-${{arch}}',
      'version': Var('dawn_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'dawn_standalone and host_os == "linux"',
  },
  'buildtools/mac': {
    'packages': [{
      'package': 'gn/gn/mac-${{arch}}',
      'version': Var('dawn_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'dawn_standalone and host_os == "mac"',
  },
  'buildtools/win': {
    'packages': [{
      'package': 'gn/gn/windows-amd64',
      'version': Var('dawn_gn_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'dawn_standalone and host_os == "win"',
  },

  'third_party/depot_tools': {
    'url': '{chromium_git}/chromium/tools/depot_tools.git@c4cdaeebba9b9ed92d041997317ffdbd042158ac',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxx.git@def01bd012f56d2fd8f5e2dc75dc28a846ab9cdb',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++abi/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxxabi.git@de3f73284c8614bb9408e45d70649f8e7933172e',
    'condition': 'dawn_standalone',
  },

  # Required by libc++
  'third_party/llvm-libc/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libc.git@885e2a900ba002ef0ba7b4757da90b0f3aa309e8',
    'condition': 'dawn_standalone',
  },

  # Required by //build on Linux
  'third_party/libdrm/src': {
    'url': '{chromium_git}/chromiumos/third_party/libdrm.git@e984d448b8b17aab853369e6c203e53719f46de1',
    'condition': 'dawn_standalone and host_os == "linux"',
  },

  # Dependencies required to use GN, and Clang in standalone.

  # The //build and //tools/* deps should all be updated in unison, as
  #  there are dependencies between them.
  'build': {
  'url': '{chromium_git}/chromium/src/build@b7873b4a4ebd2ef4fc86360629d29d195382927e',
    'condition': 'dawn_standalone',
  },
  'tools/clang': {
  'url': '{chromium_git}/chromium/src/tools/clang@de526b71081ccce1bc5fe0d8601d14487349d4a8',
    'condition': 'dawn_standalone',
  },
  'tools/memory': {
    'url': '{chromium_git}/chromium/src/tools/memory@db1b13009b4ae8a321f7055bcce8e7267ef74729',
    'condition': 'dawn_standalone',
  },
  'tools/valgrind': {
    'url': '{chromium_git}/chromium/src/tools/valgrind@22e749bda3c779dfc1b0a1dae8c7b2bee933d975',
    'condition': 'dawn_standalone',
  },
  'tools/win': {
    'url': Var('chromium_git') + '/chromium/src/tools/win@d72db9e3040567a7589e00c0ad7ddf25c3b070bc',
    'condition': 'checkout_win and not build_with_chromium',
  },
  'tools/code_coverage': {
    'url': '{chromium_git}/chromium/src/tools/code_coverage@74d04576e893e08d9d16c99866c7d6696713d554',
    'condition': 'dawn_standalone and checkout_clang_coverage_tools',
  },

  # For run-tricium-clang-tidy.py
  'third_party/chromium-tools-build/src': {
    'url': '{chromium_git}/chromium/tools/build@3e24ca20b7878d29a904c0fa0da602cfff5fb88b',
    'condition': 'dawn_standalone and checkout_clang_tidy',
  },

  # Linux sysroots for hermetic builds instead of relying on whatever is
  # available from the system used for compilation. Only applicable to
  # dawn_standalone since Chromium has its own sysroot copy.
  'build/linux/debian_bullseye_armhf-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_arm',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': 'd78a9e2739dd4cce413dc8629ad0be840591c6456f9a4c6ed4b29ecf2a4257f3',
        'sha256sum': 'd78a9e2739dd4cce413dc8629ad0be840591c6456f9a4c6ed4b29ecf2a4257f3',
        'size_bytes': 17981276,
        'generation': 1792192750740303,
      },
    ],
  },
  'build/linux/debian_bullseye_arm64-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_arm64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': '278c36fe3a785d7f89fbe3f46956899a3bd49fe665f68dc667d6f626a951fcd6',
        'sha256sum': '278c36fe3a785d7f89fbe3f46956899a3bd49fe665f68dc667d6f626a951fcd6',
        'size_bytes': 18015920,
        'generation': 1792192630237696,
      },
    ],
  },
  'build/linux/debian_bullseye_i386-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and (checkout_x86 or checkout_x64)',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': '854e63992a633c8d54b69959d7b789453239d7f4bc30216594f0e3eb1b520402',
        'sha256sum': '854e63992a633c8d54b69959d7b789453239d7f4bc30216594f0e3eb1b520402',
        'size_bytes': 19375240,
        'generation': 1792192792939606,
      },
    ],
  },
  'build/linux/debian_bullseye_mipsel-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_mips',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': 'c52c047102757452c2d740c9d322d56256edd942033a566d0fd242751e4c6fd4',
        'sha256sum': 'c52c047102757452c2d740c9d322d56256edd942033a566d0fd242751e4c6fd4',
        'size_bytes': 18186196,
        'generation': 1792192740132829,
      },
    ],
  },
  'build/linux/debian_bullseye_mips64el-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_mips64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': '932ee790d10a9a138da669ddff036de8f59111db077a12597f9a0bc062d14b94',
        'sha256sum': '932ee790d10a9a138da669ddff036de8f59111db077a12597f9a0bc062d14b94',
        'size_bytes': 18607664,
        'generation': 1792192782030052,
      },
    ],
  },
  'build/linux/debian_bullseye_amd64-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_x64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': 'f851add76bdae472fb6c94b4f920bc4a132d85ac607951391297f636af7e934e',
        'sha256sum': 'f851add76bdae472fb6c94b4f920bc4a132d85ac607951391297f636af7e934e',
        'size_bytes': 19314716,
        'generation': 1792192152972543,
      },
    ],
  },

  # Used for Dawn-side GN arg definitions.
  'tools/mb': {
    'url': '{chromium_git}/chromium/src/tools/mb@c7a91093bfa9c745ccd53c16b7789227967c7857',
    'condition': 'dawn_standalone',
  },

  # Testing, GTest and GMock
  'testing': {
    'url': '{chromium_git}/chromium/src/testing@1bddaaaaf5f0a9c79cb02a82149bd5a514a740a7',
    'condition': 'dawn_standalone',
  },
  'third_party/libFuzzer/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/compiler-rt/lib/fuzzer.git' + '@' + 'c7ea90fb61105752fa78e257e77a7c5d62fff7d3',
    'condition': 'dawn_standalone',
  },
  'third_party/googletest/src': {
    'url': '{chromium_git}/external/github.com/google/googletest@6450691fd7d07ef932582206fbafb1a513927e08',
    'condition': 'dawn_standalone',
  },
  # This is a dependency of //testing
  'third_party/catapult': {
    'url': '{chromium_git}/catapult.git@41561ed5d95bf57917114f37e1523b33c4e031f1',
    'condition': 'dawn_standalone',
  },
  'third_party/google_benchmark/src': {
    'url': '{chromium_git}/external/github.com/google/benchmark.git' + '@' + 'a6b4a9f00b36c19af9997d68a1cf6e0b5b3d032c',
    'condition': 'dawn_standalone',
  },
  'third_party/perfetto': {
    'url': '{chromium_git}/external/github.com/google/perfetto.git@b6533c5df1422eae58d2d3212510d47483d2631c',
    'condition': 'dawn_standalone',
  },

  # Required for fuzzing Mesa via tint fuzzers
  'third_party/mesa/src': {
    'url': '{chromium_git}/external/gitlab.freedesktop.org/mesa/mesa/@28806e5e824507f818a43faa422f0ebfc02603fe',
    'condition': 'checkout_mesa and host_os == "linux"',
  },
  'third_party/llvm-dev': {
    'packages': [{
      'package': 'infra/3pp/tools/llvm-dev/linux-amd64',
      'version': Var('dawn_llvm-dev_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'checkout_mesa and host_os == "linux"',
  },
  'third_party/zstd': {
    'packages': [{
      'package': 'infra/3pp/static_libs/libzstd/linux-amd64',
      'version': Var('dawn_zstd_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'checkout_mesa and host_os == "linux"',
  },

  # Jinja2 and MarkupSafe for the code generator
  'third_party/jinja2': {
    'url': '{chromium_git}/chromium/src/third_party/jinja2@fde3c16799bd5937e4f15aedc9037c89f96236b9',
    'condition': 'dawn_standalone',
  },
  'third_party/markupsafe': {
    'url': '{chromium_git}/chromium/src/third_party/markupsafe@4256084ae14175d38a3ff7d739dca83ae49ccec6',
    'condition': 'dawn_standalone',
  },

  # GLFW for tests and samples
  'third_party/glfw3/src': {
    'url': '{chromium_git}/external/github.com/glfw/glfw@92dcf4ce74f2e2554a98fea09be7c705c17daa5a',
  },

  'third_party/vulkan_memory_allocator': {
    'url': '{chromium_git}/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@7e55b011e16182fc349149abbd3aaf3b1db46421',
    'condition': 'dawn_standalone',
  },

  'third_party/angle': {
    'url': '{chromium_git}/angle/angle@615aa21abb10399ba44e9b8df1b9e33b952d9db6',
    'condition': 'dawn_standalone',
  },

  'third_party/swiftshader': {
    'url': '{swiftshader_git}/SwiftShader@708e9e77773860bbb2ad9bb2d61b451de4b84e31',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-deps': {
    'url': '{chromium_git}/vulkan-deps@660bf91094b915981a3748b26ba096db48c6459c',
    'condition': 'dawn_standalone',
  },

  'third_party/glslang/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/glslang@ff6ad3c77a2cc2fdea329b4a1d5425c82bc56a1f',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Headers@6efe5ae6b7c6ad29da3f2cc3f164b9c3d72b78b1',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Tools@251c437ef506157c139f77f5525c02223bc78868',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Headers@88487f687f3100ebe1d3559a8a34d971ee85b977',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-loader/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Loader@0a55950deb11bb5e9dd86c7260403bfd29ac678e',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Tools@1d3fbab1ff5abce9f3eb9f3e7fcf66f455ae4475',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-utility-libraries/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Utility-Libraries@5a873d3b75d38ef6a0833589ce3603d63f3f6a9c',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-validation-layers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-ValidationLayers@13e1d19f50d0c645408a128fec1718ce85cc9a3e',
    'condition': 'dawn_standalone',
  },

  'third_party/zlib': {
    'url': '{chromium_git}/chromium/src/third_party/zlib@ec453dc9abac1ddf39ced752e72a7a80dcda1d91',
    'condition': 'dawn_standalone',
  },

  'third_party/abseil-cpp': {
    'url': '{chromium_git}/chromium/src/third_party/abseil-cpp@d8ad4495323e7b20443e7129fcbfd7ee78282818',
    'condition': 'dawn_standalone',
  },

  'third_party/directx-shader-compiler/src': {
    'url': '{chromium_git}/external/github.com/microsoft/DirectXShaderCompiler@c8611f6ff76ef0c8bcd1420c4d6defe304d2658c',
  },

  'third_party/directx-headers/src': {
    # The non-Windows build of DXC depends on DirectX-Headers, and at a specific commit (not ToT)
    'url': '{chromium_git}/external/github.com/microsoft/DirectX-Headers@980971e835876dc0cde415e8f9bc646e64667bf7',
    'condition': 'host_os != "win"',
  },

  'third_party/agility-sdk/src': {
    'packages': [
      {
        'package': 'chromium/third_party/agility-sdk',
        'version': Var('dawn_agility_sdk_version'),
      },
    ],
    'condition': 'checkout_win',
    'dep_type': 'cipd',
  },

  'third_party/direct3d-warp/bin/': {
    'packages': [
      {
        'package': 'infra/3pp/direct3d-warp',
        'version': Var('dawn_direct3d_warp_version'),
      },
    ],
    'condition': 'dawn_standalone and checkout_win',
    'dep_type': 'cipd',
  },

  'third_party/OpenGL-Registry/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/OpenGL-Registry@6af574a14089ccfee87efe230ebcdd8742859813',
  },

  'third_party/EGL-Registry/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/EGL-Registry@8e56e86e65687697ef54114cdb1afabdfac756bf',
  },

  # WebGPU CTS - Used both by the dawn_node tests and transitively by Chromium.
  'third_party/webgpu-cts': {
    'url': '{chromium_git}/external/github.com/gpuweb/cts@3cc5b73f1c3f1c568bcebd3cbf069cc1aa90a931',
    'condition': 'build_with_chromium or dawn_standalone',
  },

  # Dependencies required to build / run WebAssembly bindings
  'third_party/emsdk': {
    # Note: Always use an emsdk hash referring to a tagged release, just so
    # emsdk and emscripten are always in sync with an exact release.
    'url': '{chromium_git}/external/github.com/emscripten-core/emsdk.git@948c31acd3f369a5da276e33ab2ed57108c165e5',
    'condition': 'dawn_wasm',
  },

  # Dependencies required to build / run Dawn NodeJS bindings
  'third_party/node-api-headers': {
    'url': '{chromium_git}/external/github.com/nodejs/node-api-headers@d5cfe19da8b974ca35764dd1c73b91d57cd3c4ce',
    'condition': 'dawn_node',
  },
  'third_party/node-addon-api': {
    'url': '{chromium_git}/external/github.com/nodejs/node-addon-api@1e26dcb52829a74260ec262edb41fc22998669b6',
    'condition': 'dawn_node',
  },
  'third_party/gpuweb': {
    'url': '{chromium_git}/external/github.com/gpuweb/gpuweb@22507c95ccb9e705eddc7cab8f28aa46b134f5fe',
    'condition': 'dawn_node',
  },

  # Node binaries, when dawn_node or dawn_wasm is enabled. Architectures are
  # listed out explicitly instead of using ${{platform}} because the
  # architecture that these are fetched on can differ from the architecture that
  # tests are ultimately run on, such as x64 vs. ARM64 Mac.
  'third_party/node/linux-amd64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/linux-amd64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_linux and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },
  'third_party/node/linux-arm64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/linux-arm64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_linux and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },
  'third_party/node/mac-amd64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/mac-amd64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_mac and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },
  'third_party/node/mac-arm64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/mac-arm64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_mac and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },
  'third_party/node/windows-amd64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/windows-amd64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_win and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },
  'third_party/node/windows-arm64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/nodejs/windows-arm64',
        'version': Var('dawn_node_version'),
      },
    ],
    'condition': 'checkout_win and (dawn_node or dawn_wasm)',
    'dep_type': 'cipd',
  },

  # Upstream webgpu.h headers for testing purposes
  'third_party/webgpu-headers/src': {
    'url': '{chromium_git}/external/github.com/webgpu-native/webgpu-headers@a1dc77409bb2b8bd80540cbf4b8db219e123e909',
  },

  # Like the Node dependency, architectures are listed out explicitly instead of
  # using ${{platform}} because the architecture that these are fetched on
  # can differ from the architecture that they are ultimately run on.
  'tools/golang/linux-amd64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/linux-amd64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': '(checkout_android or checkout_linux) and non_git_source',
  },
  'tools/golang/linux-arm64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/linux-arm64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': '(checkout_android or checkout_linux) and non_git_source',
  },
  'tools/golang/mac-amd64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/mac-amd64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': '(checkout_ios or checkout_mac) and non_git_source',
  },
  'tools/golang/mac-arm64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/mac-arm64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': '(checkout_ios or checkout_mac) and non_git_source',
  },
  'tools/golang/windows-amd64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/windows-amd64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'checkout_win and non_git_source',
  },
  'tools/golang/windows-arm64': {
    'packages': [{
      'package': 'infra/3pp/tools/go/windows-arm64',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
    'condition': 'checkout_win and non_git_source',
  },

  'tools/cmake': {
    'condition': '(fetch_cmake or dawn_node)',
    'packages': [{
      'package': 'infra/3pp/tools/cmake/${{platform}}',
      'version': Var('dawn_cmake_version'),
    }],
    'dep_type': 'cipd',
  },

  'tools/bazelisk': {
    'condition': 'fetch_bazel or checkout_litert_lm',
    'packages': [{
      'package': 'infra/3pp/tools/bazelisk/${{platform}}',
      'version': Var('dawn_bazelisk_version'),
    }],
    'dep_type': 'cipd',
  },

  'third_party/ninja': {
    'packages': [
      {
        'package': 'infra/3pp/tools/ninja/${{platform}}',
        'version': Var('dawn_ninja_version'),
      }
    ],
    'dep_type': 'cipd',
  },
  'third_party/siso/cipd': {
    'packages': [
      {
        'package': 'build/siso/${{platform}}',
        'version': Var('siso_version'),
      }
    ],
    'condition': 'dawn_standalone and non_git_source',
    'dep_type': 'cipd',
  },

  # Always download Linux x64 package regardless of host OS for RBE workers.
  'third_party/cpython3/linux-amd64': {
    'packages': [
      {
        'package': 'infra/3pp/tools/cpython3/linux-amd64',
        'version': Var('cpython3_version'),
      }
    ],
    'condition': 'dawn_standalone and non_git_source',
    'dep_type': 'cipd',
  },
  # Host platform package.
  'third_party/cpython3/host': {
    'packages': [
      {
        'package': 'infra/3pp/tools/cpython3/${{platform}}',
        'version': Var('cpython3_version'),
      }
    ],
    'condition': 'dawn_standalone and non_git_source',
    'dep_type': 'cipd',
  },

  # Misc dependencies inherited from Tint
  'third_party/protobuf': {
    'url': '{chromium_git}/chromium/src/third_party/protobuf@1c9b6be005a5f855d1c8dd3c63a11f2ee78f956c',
    'condition': 'dawn_standalone',
  },

  'tools/protoc_wrapper': {
    'url': '{chromium_git}/chromium/src/tools/protoc_wrapper@5c77c0c9fdebb2d87378bcea183e4c6937fd8c53',
    'condition': 'dawn_standalone',
  },

  'third_party/libprotobuf-mutator/src': {
    'url': '{chromium_git}/external/github.com/google/libprotobuf-mutator.git@a0448e5055e4740774e00d496f016d9fcd572200',
    'condition': 'dawn_standalone',
  },

  # Dependencies for PartitionAlloc.
  # Doc: https://docs.google.com/document/d/1wz45t0alQthsIU9P7_rQcfQyqnrBMXzrOjSzdQo-V-A
  'third_party/partition_alloc': {
    'url': '{chromium_git}/chromium/src/base/allocator/partition_allocator.git@ed4dc364672b67eee2acf2618c72d73cb758ab2e',
    'condition': 'dawn_standalone',
  },

  # For Android builds in standalone.
  'third_party/android_sdk': {
    'url': Var('chromium_git') + '/chromium/src/third_party/android_sdk@c4d16bfe5cf8720e6dba6f571110deb14a51d6cf',
    'condition': 'checkout_android and dawn_standalone',
  },
  'third_party/android_sdk/public': {
    'packages': [
      {
        'package': 'chromium/third_party/android_sdk/public/build-tools/37.0.0/${{os}}',
        'version': Var('android_sdk_build-tools_version'),
      },
      {
        'package': 'chromium/third_party/android_sdk/public/emulator',
        'version': Var('android_sdk_emulator_version'),
      },
      {
        'package': 'chromium/third_party/android_sdk/public/platform-tools',
        'version': Var('android_sdk_platform-tools_version'),
      },
      {
        'package': 'chromium/third_party/android_sdk/public/platforms/android-37.2',
        'version': Var('android_sdk_platforms_version'),
      },
      {
        'package': 'chromium/third_party/android_sdk/public/cmdline-tools/${{os}}',
        'version': 'version:23.0.0',
      },
    ],
    'condition': 'checkout_android and dawn_standalone',
    'dep_type': 'cipd',
  },
  'third_party/android_toolchain/ndk': {
    'packages': [
      {
        'package': 'chromium/third_party/android_toolchain/android_toolchain/${{platform}}',
        'version': 'version:' + Var('android_ndk_version'),
      },
    ],
    'condition': 'checkout_android and dawn_standalone and '
                 '((host_os == "linux" and host_cpu == "x64") or '
                 '(host_os == "mac" and host_cpu == "arm64"))',
    'dep_type': 'cipd',
  },
  'third_party/jdk/current': {
    'packages': [
      {
        'package': 'chromium/third_party/jdk/${{platform}}',
        'version': 'version:2@jdk-25.0.5+8.f32c55fd1a.cr0',
      },
    ],
    'condition': 'checkout_android and dawn_standalone and non_git_source and '
                 '((host_os == "linux" and host_cpu == "x64") or '
                 '(host_os == "mac" and host_cpu == "arm64"))',
    'dep_type': 'cipd',
  },
  'third_party/android_build_tools': {
    'url': Var('chromium_git') + '/chromium/src/third_party/android_build_tools@f8e6c884910fd20901ad38118f32b122650c72d2',
    'condition': 'checkout_android and dawn_standalone',
  },
  'third_party/android_build_tools/aapt2/cipd': {
    'packages': [
      {
        'package': 'chromium/third_party/android_build_tools/aapt2/${{platform}}',
        'version': 'version:2@9.6.0-alpha01-16666295',
      },
    ],
    'condition': 'checkout_android and dawn_standalone',
    'dep_type': 'cipd',
  },
  'third_party/android_build_tools/manifest_merger/cipd': {
    'packages': [
      {
        'package': 'chromium/third_party/android_build_tools/manifest_merger',
        'version': 'r210EZVHGOLD6PgqyMpRe9DHMckVbi4c15VXmfQaLskC',
      },
    ],
    'condition': 'checkout_android and dawn_standalone',
    'dep_type': 'cipd',
  },
  'third_party/cpu_features/src': {
    'url': Var('chromium_git') + '/external/github.com/google/cpu_features.git' + '@' + '81d13c49649f0714dd41fb56bb246398b6584085',
    'condition': 'checkout_android and dawn_standalone',
  },
  'third_party/ijar': {
    'url': Var('chromium_git') + '/chromium/src/third_party/ijar@94af60a05b33f9acb33477a8d969e48eb1c3029f',
    'condition': 'checkout_android and dawn_standalone',
  },
  'third_party/libunwind/src': {
    'url': Var('chromium_git') + '/external/github.com/llvm/llvm-project/libunwind.git@9e44ea2ff4f53a60c2a3d2b6f511d7bab9ab12f0',
    'condition': 'checkout_android and dawn_standalone',
  },

  # We never want to actually checkout Chromium, but we need a fake DEPS entry
  # in order for the Chromium -> Dawn DEPS autoroller to work. Note that this
  # only currently works due to an explicit exception in presubmit checks
  # https://source.chromium.org/chromium/chromium/tools/depot_tools/+/dac161882feaedaabcf99db47726a999e4834a13:presubmit_canned_checks.py;l=2139
  'third_party/placeholder_chromium': {
    'url': '{chromium_git}/chromium/src.git' + '@' + Var('chromium_revision'),
    'condition': 'checkout_placeholder_chromium',
  },

  'third_party/litert-lm/src': {
    'url': '{chromium_git}/external/github.com/google-ai-edge/LiteRT-LM.git@2f8284d5ea323278989facfaaed1c476b174760a',
    'condition': 'checkout_litert_lm',
  },

  'third_party/litert-lm/data': {
    'packages': [
      {
        # TODO(crbug.com/527944617): Replace experimental CIPD dependency.
        'package': 'experimental/chouinard_at_google.com/litert_lm_benchmark_data',
 'version': 'MfFYLUhIzajgKbb0NIc_KPoe5_r8qf9dwAi_Ndgy-nMC',
      }
    ],
    'dep_type': 'cipd',
    'condition': 'checkout_litert_lm',
  },
  'third_party/litert-lm/models/gemma4_2b': {
    'packages': [
      {
        # TODO(crbug.com/527944617): Replace experimental CIPD dependency.
        'package': 'experimental/ohjerry_at_google.com/gemma4_2b',
        'version': '8BbbFJuxlNAt7We_SL3apsoXv9KF3tOoQXGp0DNFQE8C',
      }
    ],
    'dep_type': 'cipd',
    'condition': 'checkout_litert_lm',
  },

  # V8 and its dependencies for standalone builds.
  'third_party/v8/src': {
    'url': '{chromium_git}/v8/v8.git@a9909cbabd9f099ba5f5d6e4010c4cba104ec93a',
    'condition': 'dawn_standalone and checkout_v8',
  },

  'third_party/fp16/src': {
    'url': '{chromium_git}/external/github.com/Maratyszcza/FP16.git@782eea126dc5c755827be751a099eb01826175cf',
    'condition': 'dawn_standalone and checkout_v8',
  },

  'third_party/fast_float/src': {
    'url': '{chromium_git}/external/github.com/fastfloat/fast_float.git@dbeabeeb3e6791cb96994c2a7a084c36e62ccfb2',
    'condition': 'dawn_standalone and checkout_v8',
  },

  'third_party/fadec/src': {
    'url': '{chromium_git}/external/github.com/aengelke/fadec.git@c9f78f532b9004de278489019ab2c6c28ae9746e',
    'condition': 'dawn_standalone and checkout_v8',
  },

  'third_party/highway/src': {
    'url': '{chromium_git}/external/github.com/google/highway.git@2607d3b5b0113992fe84d3848859eae13b3b52c1',
    'condition': 'dawn_standalone and checkout_v8',
  },

  'third_party/dragonbox/src': {
    'url': '{chromium_git}/external/github.com/jk-jeon/dragonbox.git@beeeef91cf6fef89a4d4ba5e95d47ca64ccb3a44',
    'condition': 'dawn_standalone and checkout_v8',
  },

  'third_party/simdutf/src': {
    'url': '{chromium_git}/chromium/src/third_party/simdutf@f7356eed293f8208c40b3c1b344a50bd70971983',
    'condition': 'dawn_standalone and checkout_v8',
  },

  'third_party/boringssl/src': {
    'url': '{boringssl_git}/boringssl.git@0131ae9d78e42fcc2c1811315e62410ad96d8230',
    'condition': 'dawn_standalone and checkout_v8',
  },

  'third_party/nasm/src': {
    'url': '{chromium_git}/chromium/deps/nasm.git@525a09a813be0f75b646ee93fc2a31c27b87d722',
    'condition': 'dawn_standalone and checkout_v8',
  },

  'third_party/llvm-libclang': {
    'dep_type': 'gcs',
    'bucket': 'chromium-browser-clang',
    'condition': 'dawn_standalone and checkout_v8',
    'objects': [
      {
        'object_name': 'Linux_x64/rust-libclang-96a18bf5ffa2b24f9cf3d1c731a9eb121c1acad9-4-llvmorg-24-init-17044-gc3223914.tar.xz',
        'sha256sum': 'a111564c0a12e67af48bfb093377671cc453a8d6a4308fc4822a37b44556c51a',
        'size_bytes': 23178056,
        'generation': 1793896096203442,
        'condition': 'host_os == "linux" and non_git_source',
      },
      {
        'object_name': 'Mac/rust-libclang-96a18bf5ffa2b24f9cf3d1c731a9eb121c1acad9-4-llvmorg-24-init-17044-gc3223914.tar.xz',
        'sha256sum': '5a8f21ec37ad3c6d7f8d0fd190f0be50aa3f6ff757535aa685bb6499a79c451d',
        'size_bytes': 23331568,
        'generation': 1793896100021022,
        'condition': 'host_os == "mac" and host_cpu == "x64"',
      },
      {
        'object_name': 'Mac_arm64/rust-libclang-96a18bf5ffa2b24f9cf3d1c731a9eb121c1acad9-4-llvmorg-24-init-17044-gc3223914.tar.xz',
        'sha256sum': '47e232e3e635f84d41ddaa5d949ae312b813c8dfaf64264ffc424e7f44763839',
        'size_bytes': 20996496,
        'generation': 1793896104170962,
        'condition': 'host_os == "mac" and host_cpu == "arm64"',
      },
      {
        'object_name': 'Win/rust-libclang-96a18bf5ffa2b24f9cf3d1c731a9eb121c1acad9-4-llvmorg-24-init-17044-gc3223914.tar.xz',
        'sha256sum': '83e53bfa846e7f810ca7d96d0872ad5535163f9259c464d867de2cd97e8a7672',
        'size_bytes': 21908536,
        'generation': 1793896108514779,
        'condition': 'host_os == "win"',
      },
    ],
  },
}

hooks = [
  {
    # Ensure that the DEPS'd "depot_tools" has its self-update capability
    # disabled.
    'name': 'disable_depot_tools_selfupdate',
    'pattern': '.',
    'condition': 'dawn_standalone',
    'action': [
        'vpython3',
        'third_party/depot_tools/update_depot_tools_toggle.py',
        '--disable',
    ],
  },

  # Pull the compilers and system libraries for hermetic builds
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_linux and (checkout_x86 or checkout_x64)',
    'action': ['vpython3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_linux and checkout_x64',
    'action': ['vpython3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    # Update the Mac toolchain if possible, this makes builders use "hermetic XCode" which is
    # is more consistent (only changes when rolling build/) and is cached.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_mac',
    'action': ['vpython3', 'build/mac_toolchain.py'],
  },
  {
    # Create the required ranlib symlink for hermetic xcode
    'name': 'mac_ranlib_symlink',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_mac',
    'action': ['vpython3', 'tools/create_ranlib_symlink.py'],
  },
  {
    # Case-insensitivity for the Win SDK. Must run before win_toolchain below.
    'name': 'ciopfs_linux',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "linux"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang/ciopfs',
                '-s', 'build/ciopfs.sha1',
    ]
  },
  {
    # Update the Windows toolchain if necessary. Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win',
    'action': ['vpython3', 'build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['vpython3', 'tools/clang/scripts/update.py',
               '--package=clang'],
    'condition': 'dawn_standalone',
  },
  {
    # This is also supposed to support the same set of platforms as 'clang'
    # above. LLVM ToT support isn't provided at the moment.
    'name': 'clang_tidy',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_clang_tidy',
    'action': ['vpython3', 'tools/clang/scripts/update.py',
               '--package=clang-tidy'],
  },
  {
    # This is also supposed to support the same set of platforms as 'clang'
    # above. LLVM ToT support isn't provided at the moment.
    'name': 'clangd',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_clangd',
    'action': ['vpython3', 'tools/clang/scripts/update.py',
               '--package=clangd'],
  },
  {
    'name': 'objdump',
    'pattern': '.',
    'action': ['vpython3', 'tools/clang/scripts/update.py',
               '--package=objdump'],
    'condition': 'dawn_standalone',
  },
  {
    'name': 'coverage_tools',
    'pattern': '.',
    'action': ['vpython3', 'tools/clang/scripts/update.py',
               '--package=coverage_tools'],
    'condition': 'dawn_standalone and checkout_clang_coverage_tools',
  },

  # Pull dsymutil binaries using checked-in hashes.
  {
    'name': 'dsymutil_mac_arm64',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "mac" and host_cpu == "arm64"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang',
                '-s', 'tools/clang/dsymutil/bin/dsymutil.arm64.sha1',
                '-o', 'tools/clang/dsymutil/bin/dsymutil',
    ],
  },
  {
    'name': 'dsymutil_mac_x64',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "mac" and host_cpu == "x64"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang',
                '-s', 'tools/clang/dsymutil/bin/dsymutil.x64.sha1',
                '-o', 'tools/clang/dsymutil/bin/dsymutil',
    ],
  },
  # Pull rc binaries using checked-in hashes.
  {
    'name': 'rc_win',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "win"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'build/toolchain/win/rc/win/rc.exe.sha1',
    ],
  },
  {
    'name': 'rc_linux',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "linux"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'build/toolchain/win/rc/linux64/rc.sha1',
    ],
  },
  {
    'name': 'rc_mac',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "mac"',
    'action': [ 'vpython3',
                'third_party/depot_tools/download_from_google_storage.py',
                '--no_resume',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'build/toolchain/win/rc/mac/rc.sha1',
    ],
  },

  # Update build/util/LASTCHANGE.
  {
    'name': 'lastchange',
    'pattern': '.',
    'condition': 'dawn_standalone',
    'action': ['vpython3', 'build/util/lastchange.py',
               '-o', 'build/util/LASTCHANGE'],
  },

  # Activate emsdk for WebAssembly builds
  {
    'name': 'activate_emsdk',
    'pattern': '.',
    'condition': 'dawn_wasm',
    'action': [ 'vpython3', 'tools/activate-emsdk' ],
  },

  # Configure remote exec cfg files
  {
    'name': 'download_and_configure_reclient_cfgs',
    'pattern': '.',
    'condition': 'dawn_standalone and download_remoteexec_cfg',
    'action': ['vpython3',
               'buildtools/reclient_cfgs/configure_reclient_cfgs.py',
               '--rbe_instance',
               Var('rbe_instance'),
               '--rewrapper_cfg_project',
               Var('rewrapper_cfg_project'),
               '--quiet',
               ],
  },
  {
    'name': 'configure_reclient_cfgs',
    'pattern': '.',
    'condition': 'dawn_standalone and not download_remoteexec_cfg',
    'action': ['vpython3',
               'buildtools/reclient_cfgs/configure_reclient_cfgs.py',
               '--rbe_instance',
               Var('rbe_instance'),
               '--rewrapper_cfg_project',
               Var('rewrapper_cfg_project'),
               '--skip_remoteexec_cfg_fetch',
               '--quiet',
               ],
  },
  # Configure Siso for developer builds.
  {
    'name': 'configure_siso',
    'pattern': '.',
    'condition': 'dawn_standalone',
    'action': ['vpython3',
               'build/config/siso/configure_siso.py',
               '--rbe_instance',
               Var('rbe_instance'),
               ],
  },
]

recursedeps = [
  'buildtools',
]
