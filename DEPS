use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

gclient_gn_args = [
  'build_with_chromium',
  'dawn_node',
  'dawn_wasm',
  'generate_location_tags',
]

git_dependencies = 'SYNC'

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'dawn_git': 'https://dawn.googlesource.com',
  'github_git': 'https://github.com',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'dawn_standalone': True,
  'dawn_node': False, # Also fetches dependencies required for building NodeJS bindings.
  'dawn_wasm': False, # Also fetches dependencies required for building WebAssembly.
  'dawn_tintd': False, # Also fetches dependencies required for building tintd.
  'dawn_cmake_version': 'version:2@3.23.3',
  'dawn_cmake_win32_sha1': 'b106d66bcdc8a71ea2cdf5446091327bfdb1bcd7',
  'dawn_gn_version': 'git_revision:c3f29c909a6255857093a797b9360b36f931b1cb',
  # ninja CIPD package version.
  # https://chrome-infra-packages.appspot.com/p/infra/3pp/tools/ninja
  'dawn_ninja_version': 'version:3@1.12.1.chromium.4',
  'dawn_go_version': 'version:3@1.25.0',
  'dawn_node_version': 'version:2@20.11.0',
  'agility_sdk_version': 'version:2@1.721.0-preview',
  'dawn_bazelisk_version': 'version:3@1.29.0',

  # GN variable required by //testing that will be output in the gclient_args.gni
  'generate_location_tags': False,

  # Fetch clang-tidy into the same bin/ directory as our clang binary.
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
  # reclient CIPD package
  'reclient_package': 'infra/rbe/client/',
  # reclient CIPD package version
  'reclient_version': 're_client_version:0.185.0.db415f21-gomaip',
  # siso CIPD package version.
  'siso_version': 'git_revision:0b5eb879ba9260eb6fd7dfe8f9437ef3396185c4',

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
  'chromium_revision': '0002528a4c5c94f85a000e2773d4cf079d8da021',
  # We never want to actually checkout Chromium, but we need a fake DEPS entry
  # in order for the Chromium -> Dawn DEPS autoroller to work.
  'checkout_placeholder_chromium': False,

  # Checkout mesa 3D graphics library and related dependencies
  # Not used by Dawn/Tint directly, but is used as part of extended fuzzing stack.
  'checkout_mesa': False,

  # Checkout //tools/code_coverage from Chromium and fetches the
  # prebuilt versions of llvm-cov and llvm-profdata.
  'checkout_clang_coverage_tools': False,
}

deps = {
  'buildtools': {
    'url': '{chromium_git}/chromium/src/buildtools@0d39be5a3f129cf1f35e7812108a2184e2193315',
    'condition': 'dawn_standalone',
  },
  'third_party/clang-format/script': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@6eddfb5ec5f92127a531eda66c568d3a11e7ec11',
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
    'url': '{chromium_git}/chromium/tools/depot_tools.git@f710e50c6e928811caca1139e997d9194018ed6e',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxx.git@5abc7f839700f0f17338434e1c1c6a8c87c00c11',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++abi/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxxabi.git@8f11bb1d4438d0239d0dfc1bd9456a9f31629dda',
    'condition': 'dawn_standalone',
  },

  # Required by libc++
  'third_party/llvm-libc/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libc.git@b57686f74130d6e59939431111ac2f778ef503d5',
    'condition': 'dawn_standalone',
  },

  # Required by //build on Linux
  'third_party/libdrm/src': {
    'url': '{chromium_git}/chromiumos/third_party/libdrm.git@369990d9660a387f618d0eedc341eb285016243b',
    'condition': 'dawn_standalone and host_os == "linux"',
  },

  # Dependencies required to use GN, and Clang in standalone.

  # The //build and //tools/* deps should all be updated in unison, as
  #  there are dependencies between them.
  'build': {
  'url': '{chromium_git}/chromium/src/build@aa51416c54c44310653355b54520eda5cbb716e5',
    'condition': 'dawn_standalone',
  },
  'tools/clang': {
  'url': '{chromium_git}/chromium/src/tools/clang@dd7362d6df176da8001a8195597968939b444ab1',
    'condition': 'dawn_standalone',
  },
  'tools/memory': {
    'url': '{chromium_git}/chromium/src/tools/memory@a7e928b8bb8d79aa2feb809c1bd4752eecc68802',
    'condition': 'dawn_standalone',
  },
  'tools/valgrind': {
    'url': '{chromium_git}/chromium/src/tools/valgrind@da34b95fdbf2032df6cda5f3828c2ba421592644',
    'condition': 'dawn_standalone',
  },
  'tools/win': {
    'url': Var('chromium_git') + '/chromium/src/tools/win@faefd1b6fa9eeb033ad6fe60368ccb9bf908cbd0',
    'condition': 'checkout_win and not build_with_chromium',
  },
  'tools/code_coverage': {
    'url': '{chromium_git}/chromium/src/tools/code_coverage@74d04576e893e08d9d16c99866c7d6696713d554',
    'condition': 'dawn_standalone and checkout_clang_coverage_tools',
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
        'object_name': 'b45a7f586a107380ca6141b00d74321922b41d6d327dc33e74a2f82fd454304c',
        'sha256sum': 'b45a7f586a107380ca6141b00d74321922b41d6d327dc33e74a2f82fd454304c',
        'size_bytes': 18374340,
        'generation': 1770327986819219,
      },
    ],
  },
  'build/linux/debian_bullseye_arm64-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_arm64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': 'c7176a4c7aacbf46bda58a029f39f79a68008d3dee6518f154dcf5161a5486d8',
        'sha256sum': 'c7176a4c7aacbf46bda58a029f39f79a68008d3dee6518f154dcf5161a5486d8',
        'size_bytes': 18420984,
        'generation': 1770327978874031,
      },
    ],
  },
  'build/linux/debian_bullseye_i386-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and (checkout_x86 or checkout_x64)',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': '3de724b0d63478e1ae35f07b95d02261581a66e05c19aebe4e443d76179a565e',
        'sha256sum': '3de724b0d63478e1ae35f07b95d02261581a66e05c19aebe4e443d76179a565e',
        'size_bytes': 19768196,
        'generation': 1770327987132454,
      },
    ],
  },
  'build/linux/debian_bullseye_mipsel-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_mips',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': '82e930d6fa5d5ab1172cabc63b911ec800b182b4f8c14a273a89596541fe8658',
        'sha256sum': '82e930d6fa5d5ab1172cabc63b911ec800b182b4f8c14a273a89596541fe8658',
        'size_bytes': 18613672,
        'generation': 1770327971826284,
      },
    ],
  },
  'build/linux/debian_bullseye_mips64el-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_mips64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': 'c847a32ae492aa14688be47fa696026e8dae8d9f4f589ec42fad29862bf311b3',
        'sha256sum': 'c847a32ae492aa14688be47fa696026e8dae8d9f4f589ec42fad29862bf311b3',
        'size_bytes': 19042256,
        'generation': 1770327970830699,
      },
    ],
  },
  'build/linux/debian_bullseye_amd64-sysroot': {
    'bucket': 'chrome-linux-sysroot',
    'condition': 'dawn_standalone and checkout_linux and checkout_x64',
    'dep_type': 'gcs',
    'objects': [
      {
        'object_name': '52d61d4446ffebfaa3dda2cd02da4ab4876ff237853f46d273e7f9b666652e1d',
        'sha256sum': '52d61d4446ffebfaa3dda2cd02da4ab4876ff237853f46d273e7f9b666652e1d',
        'size_bytes': 19727236,
        'generation': 1770327973518330,
      },
    ],
  },

  # Used for Dawn-side GN arg definitions.
  'tools/mb': {
    'url': '{chromium_git}/chromium/src/tools/mb@f3daf5ac4d880d4188bb2ecd9a5bb34810610de9',
    'condition': 'dawn_standalone',
  },

  # Testing, GTest and GMock
  'testing': {
    'url': '{chromium_git}/chromium/src/testing@e48d98b6879136c181ba836b7adf96334da2b459',
    'condition': 'dawn_standalone',
  },
  'third_party/libFuzzer/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/compiler-rt/lib/fuzzer.git' + '@' + 'bea408a6e01f0f7e6c82a43121fe3af4506c932e',
    'condition': 'dawn_standalone',
  },
  'third_party/googletest': {
    'url': '{chromium_git}/external/github.com/google/googletest@4fe3307fb2d9f86d19777c7eb0e4809e9694dde7',
    'condition': 'dawn_standalone',
  },
  # This is a dependency of //testing
  'third_party/catapult': {
    'url': '{chromium_git}/catapult.git@42132fc77ec562ee0ccb7db809c7ea7cc94ef294',
    'condition': 'dawn_standalone',
  },
  'third_party/google_benchmark/src': {
    'url': '{chromium_git}/external/github.com/google/benchmark.git' + '@' + '8abf1e701fbd88c8170f48fe0558247e2e5f8e7d',
    'condition': 'dawn_standalone',
  },

  # Required tint fuzzer + mesa
  'third_party/mesa/src': {
    'url': '{chromium_git}/external/gitlab.freedesktop.org/mesa/mesa/@2e683eb7385c54f872acc47b371210d2282bc103',
    'condition': 'checkout_mesa',
  },
  'third_party/meson/src': {
    'url': '{chromium_git}/external/github.com/mesonbuild/meson@d389906a136c2aac9820ded0f38d1e25ef25fb9a',
    'condition': 'checkout_mesa',
  },

  # Jinja2 and MarkupSafe for the code generator
  'third_party/jinja2': {
    'url': '{chromium_git}/chromium/src/third_party/jinja2@c3027d884967773057bf74b957e3fea87e5df4d7',
    'condition': 'dawn_standalone',
  },
  'third_party/markupsafe': {
    'url': '{chromium_git}/chromium/src/third_party/markupsafe@4256084ae14175d38a3ff7d739dca83ae49ccec6',
    'condition': 'dawn_standalone',
  },

  # GLFW for tests and samples
  'third_party/glfw3/src': {
    'url': '{chromium_git}/external/github.com/glfw/glfw@567b1ec2442d59525e24c19e8d413df6baf02496',
  },

  'third_party/vulkan_memory_allocator': {
    'url': '{chromium_git}/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@7e55b011e16182fc349149abbd3aaf3b1db46421',
    'condition': 'dawn_standalone',
  },

  'third_party/angle': {
    'url': '{chromium_git}/angle/angle@317ae3119bf1004b3307863e75b6b7d9705d0174',
    'condition': 'dawn_standalone',
  },

  'third_party/swiftshader': {
    'url': '{swiftshader_git}/SwiftShader@2843cbcc714fe111e1083127c048a18002bc10ed',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-deps': {
    'url': '{chromium_git}/vulkan-deps@4210d7fc52f637086d28e71bca666962c3bbdda9',
    'condition': 'dawn_standalone',
  },

  'third_party/glslang/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/glslang@20960a4872f681e4213312b06b48cc4ddae3c73d',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Headers@c63848ecf2200425511319fd8bf2c17b751e501e',
    'condition': 'dawn_standalone',
  },

  'third_party/spirv-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Tools@113784c9cf103b775fdadb1ef194f6b963e24a7e',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-headers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Headers@01393c3df0e5285b54ee6527466513f9e614be94',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-loader/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Loader@b52703d439c3077705db64e61c186545ddfdb2c7',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-tools/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Tools@db008c1ac735713164b498f36b796bc7f00387f3',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-utility-libraries/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Utility-Libraries@8183a0b86632bf1107553eab1e69d7b85d455477',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-validation-layers/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-ValidationLayers@4d6c2fe82482f7f9fd22538bb8bdcb4443e62272',
    'condition': 'dawn_standalone',
  },

  'third_party/zlib': {
    'url': '{chromium_git}/chromium/src/third_party/zlib@3246f1b60849cc505e231c5d19d0cbf358093555',
    'condition': 'dawn_standalone',
  },

  'third_party/abseil-cpp': {
    'url': '{chromium_git}/chromium/src/third_party/abseil-cpp@7808b6332fd5a14b7a8f7c76db9d1b78f5fc1117',
    'condition': 'dawn_standalone',
  },

  'third_party/directx-shader-compiler/src': {
    'url': '{chromium_git}/external/github.com/microsoft/DirectXShaderCompiler@363b46013d143973d69939a2ad52b186d2c77558',
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
        'version': Var('agility_sdk_version'),
      },
    ],
    'condition': 'dawn_standalone and host_os == "win"',
    'dep_type': 'cipd',
  },

  'third_party/OpenGL-Registry/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/OpenGL-Registry@9d527dbc81bb76e35ba284fe385ed8a5ddb90cbc',
  },

  'third_party/EGL-Registry/src': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/EGL-Registry@3d7796b3721d93976b6bfe536aa97bbc4bce8667',
  },

  # WebGPU CTS - Used both by the dawn_node tests and transitively by Chromium.
  'third_party/webgpu-cts': {
    'url': '{chromium_git}/external/github.com/gpuweb/cts@cf0073691abc3b97587411c105e14f0af795db37',
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
    'url': '{chromium_git}/external/github.com/gpuweb/gpuweb@acaf809d9323e72429d2252e372ee4d917fc40eb',
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
    'url': '{chromium_git}/external/github.com/webgpu-native/webgpu-headers@bf8ddb91dc38ea11ec1b727dae1fa965c4207d22',
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
    'condition': 'fetch_bazel',
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

  # RBE dependencies
  'buildtools/reclient': {
    'packages': [
      {
        'package': Var('reclient_package') + '${{platform}}',
        'version': Var('reclient_version'),
      }
    ],
    'condition': 'dawn_standalone and (host_cpu != "arm64" or host_os == "mac") and non_git_source',
    'dep_type': 'cipd',
  },

  # Misc dependencies inherited from Tint
  'third_party/protobuf': {
    'url': '{chromium_git}/chromium/src/third_party/protobuf@3f95fa8bba0e72066edf77c8977bd6700d3e7918',
    'condition': 'dawn_standalone',
  },

  'tools/protoc_wrapper': {
    'url': '{chromium_git}/chromium/src/tools/protoc_wrapper@418c65786fdf6fc5f10cb008c252c2b12c4713a6',
    'condition': 'dawn_standalone',
  },

  'third_party/libprotobuf-mutator/src': {
    'url': '{chromium_git}/external/github.com/google/libprotobuf-mutator.git@c1c950eae0440c3808f2b8bd7c57d0c6a42c1a90',
    'condition': 'dawn_standalone',
  },

  # Dependencies for tintd.
  'third_party/jsoncpp': {
    'url': '{chromium_git}/external/github.com/open-source-parsers/jsoncpp.git@800aa28c493590c539cb7baf445016cc9b8702ef',
    'condition': 'dawn_tintd',
  },

  'third_party/langsvr': {
    'url': '{github_git}/google/langsvr.git@303c526231a90049a3e384549720f3fbd453cf66',
    'condition': 'dawn_tintd',
  },

  # Dependencies for PartitionAlloc.
  # Doc: https://docs.google.com/document/d/1wz45t0alQthsIU9P7_rQcfQyqnrBMXzrOjSzdQo-V-A
  'third_party/partition_alloc': {
    'url': '{chromium_git}/chromium/src/base/allocator/partition_allocator.git@4e256afb47b4ed846f38e9653aada344613bf754',
    'condition': 'dawn_standalone',
  },

  # We never want to actually checkout Chromium, but we need a fake DEPS entry
  # in order for the Chromium -> Dawn DEPS autoroller to work. Note that this
  # only currently works due to an explicit exception in presubmit checks
  # https://source.chromium.org/chromium/chromium/tools/depot_tools/+/dac161882feaedaabcf99db47726a999e4834a13:presubmit_canned_checks.py;l=2139
  'third_party/placeholder_chromium': {
    'url': '{chromium_git}/chromium/src.git' + '@' + Var('chromium_revision'),
    'condition': 'checkout_placeholder_chromium',
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
    # Use luci_auth if on windows and using chrome-untrusted project
    'name': 'download_and_configure_reclient_cfgs',
    'pattern': '.',
    'condition': 'dawn_standalone and download_remoteexec_cfg and host_os == "win"',
    'action': ['vpython3',
               'buildtools/reclient_cfgs/configure_reclient_cfgs.py',
               '--rbe_instance',
               Var('rbe_instance'),
               '--reproxy_cfg_template',
               'reproxy.cfg.template',
               '--rewrapper_cfg_project',
               Var('rewrapper_cfg_project'),
               '--use_luci_auth_credshelper',
               '--quiet',
               ],
  },  {
    'name': 'download_and_configure_reclient_cfgs',
    'pattern': '.',
    'condition': 'dawn_standalone and download_remoteexec_cfg and not host_os == "win"',
    'action': ['vpython3',
               'buildtools/reclient_cfgs/configure_reclient_cfgs.py',
               '--rbe_instance',
               Var('rbe_instance'),
               '--reproxy_cfg_template',
               'reproxy.cfg.template',
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
               '--reproxy_cfg_template',
               'reproxy.cfg.template',
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
