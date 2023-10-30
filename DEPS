use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

gclient_gn_args = [
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
  'dawn_cmake_version': 'version:2@3.23.3',
  'dawn_cmake_win32_sha1': 'b106d66bcdc8a71ea2cdf5446091327bfdb1bcd7',
  'dawn_gn_version': 'git_revision:182a6eb05d15cc76d2302f7928fdb4f645d52c53',
  # ninja CIPD package version.
  # https://chrome-infra-packages.appspot.com/p/infra/3pp/tools/ninja
  'dawn_ninja_version': 'version:2@1.11.1.chromium.6',
  'dawn_go_version': 'version:2@1.21.3',

  'node_darwin_arm64_sha': '31859fc1fa0994a95f44f09c367d6ff63607cfde',
  'node_darwin_x64_sha': '16dfd094763b71988933a31735f9dea966f9abd6',
  'node_linux_x64_sha': 'ab9544e24e752d3d17f335fb7b2055062e582d11',
  'node_win_x64_sha': '5ef847033c517c499f56f9d136d159b663bab717',

  # GN variable required by //testing that will be output in the gclient_args.gni
  'generate_location_tags': False,

  # Fetch clang-tidy into the same bin/ directory as our clang binary.
  'checkout_clang_tidy': False,

  # Fetch the rust toolchain.
  #
  # Use a custom_vars section to enable it:
  # "custom_vars": {
  #   "checkout_rust": True,
  # }
  'checkout_rust': False,

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
  'reclient_version': 're_client_version:0.114.2.81e819b-gomaip',

  # 'magic' text to tell depot_tools that git submodules should be accepted
  # but parity with DEPS file is expected.
  'SUBMODULE_MIGRATION': 'True'
}

deps = {
  'buildtools': {
    'url': '{chromium_git}/chromium/src/buildtools@a9a6f0c49d0e8fa0cda37337430b4736ab3dc944',
    'condition': 'dawn_standalone',
  },
  'third_party/clang-format/script': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@8b525d2747f2584fc35d8c7e612e66f377858df7',
    'condition': 'dawn_standalone',
  },
  'buildtools/linux64': {
    'packages': [{
      'package': 'gn/gn/linux-amd64',
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

  'third_party/libc++/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxx.git@84fb809dd6dae36d556dc0bb702c6cc2ce9d4b80',
    'condition': 'dawn_standalone',
  },

  'third_party/libc++abi/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxxabi.git@d4760c0af99ccc9bce077960d5ddde4d66146c05',
    'condition': 'dawn_standalone',
  },

  # Dependencies required to use GN, Clang, and Rust in standalone.
  # The //build, //tools/clang, and //tools/rust deps should all be updated
  # in unison, as there are dependencies between them.
  'build': {
    'url': '{chromium_git}/chromium/src/build@e2f4d00875f7d00fad39d5af2c6869ac7c7413cc',
    'condition': 'dawn_standalone',
  },
  'tools/clang': {
    'url': '{chromium_git}/chromium/src/tools/clang@86aed39db276fb876a2b98c93cc6ff8940377903',
    'condition': 'dawn_standalone',
  },
  'tools/rust': {
    'url': '{chromium_git}/chromium/src/tools/rust@7052bd3aa0eba2d3d701b7a76219e3b04770540e',
    'condition': 'dawn_standalone and checkout_rust',
  },
  'tools/clang/dsymutil': {
    'packages': [{
      'package': 'chromium/llvm-build-tools/dsymutil',
      'version': 'M56jPzDv1620Rnm__jTMYS62Zi8rxHVq7yw0qeBFEgkC',
    }],
    'condition': 'dawn_standalone and (checkout_mac or checkout_ios)',
    'dep_type': 'cipd',
  },

  # Testing, GTest and GMock
  'testing': {
    'url': '{chromium_git}/chromium/src/testing@035a9b18047370df7403758b006e6c9696d6b84d',
    'condition': 'dawn_standalone',
  },
  'third_party/libFuzzer/src': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/compiler-rt/lib/fuzzer.git' + '@' + '26cc39e59b2bf5cbc20486296248a842c536878d',
    'condition': 'dawn_standalone',
  },
  'third_party/googletest': {
    'url': '{chromium_git}/external/github.com/google/googletest@7a7231c442484be389fdf01594310349ca0e42a8',
    'condition': 'dawn_standalone',
  },
  # This is a dependency of //testing
  'third_party/catapult': {
    'url': '{chromium_git}/catapult.git@dd218dfd815774289f8a81015f7a3131f72afbde',
    'condition': 'dawn_standalone',
  },
  'third_party/google_benchmark/src': {
    'url': '{chromium_git}/external/github.com/google/benchmark.git' + '@' + 'efc89f0b524780b1994d5dddd83a92718e5be492',
    'condition': 'dawn_standalone',
  },

  # Jinja2 and MarkupSafe for the code generator
  'third_party/jinja2': {
    'url': '{chromium_git}/chromium/src/third_party/jinja2@515dd10de9bf63040045902a4a310d2ba25213a0',
    'condition': 'dawn_standalone',
  },
  'third_party/markupsafe': {
    'url': '{chromium_git}/chromium/src/third_party/markupsafe@006709ba3ed87660a17bd4548c45663628f5ed85',
    'condition': 'dawn_standalone',
  },

  # GLFW for tests and samples
  'third_party/glfw': {
    'url': '{chromium_git}/external/github.com/glfw/glfw@62e175ef9fae75335575964c845a302447c012c7',
  },

  'third_party/vulkan_memory_allocator': {
    'url': '{chromium_git}/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@e87036508bb156f9986ea959323de1869e328f58',
    'condition': 'dawn_standalone',
  },

  'third_party/angle': {
    'url': '{chromium_git}/angle/angle@b56a72045a2b8ca5daf15d8177b462d256fc5181',
    'condition': 'dawn_standalone',
  },

  'third_party/swiftshader': {
    'url': '{swiftshader_git}/SwiftShader@d9ec9befba05a8dfca09c1e88f3f7be0e4b153c6',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-deps': {
    'url': '{chromium_git}/vulkan-deps@1a99a795c2297084596c89b559c3a3a55bbf7abf',
    'condition': 'dawn_standalone',
  },

  'third_party/zlib': {
    'url': '{chromium_git}/chromium/src/third_party/zlib@526382e41c9c5275dc329db4328b54e4f344a204',
    'condition': 'dawn_standalone',
  },

  'third_party/abseil-cpp': {
    'url': '{chromium_git}/chromium/src/third_party/abseil-cpp@4ef9b33175828ea46d091e7e5ec28259d39a8ba5',
    'condition': 'dawn_standalone',
  },

  'third_party/dxc': {
    'url': '{chromium_git}/external/github.com/microsoft/DirectXShaderCompiler@64030a4e01e27c608b6c5f1ab5add43575e086f9',
  },

  'third_party/dxheaders': {
    # The non-Windows build of DXC depends on DirectX-Headers, and at a specific commit (not ToT)
    'url': '{chromium_git}/external/github.com/microsoft/DirectX-Headers@980971e835876dc0cde415e8f9bc646e64667bf7',
    'condition': 'host_os != "win"',
  },

  'third_party/khronos/OpenGL-Registry': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/OpenGL-Registry@5bae8738b23d06968e7c3a41308568120943ae77',
  },

  'third_party/khronos/EGL-Registry': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/EGL-Registry@7dea2ed79187cd13f76183c4b9100159b9e3e071',
  },

  # WebGPU CTS - not used directly by Dawn, only transitively by Chromium.
  'third_party/webgpu-cts': {
    'url': '{chromium_git}/external/github.com/gpuweb/cts@2ef3f322027bec1bb5572f5083d478148d355d79',
    'condition': 'build_with_chromium',
  },

  # Dependencies required to build / run Dawn NodeJS bindings
  'third_party/node-api-headers': {
    'url': '{github_git}/nodejs/node-api-headers.git@d68505e4055ecb630e14c26c32e5c2c65e179bba',
    'condition': 'dawn_node',
  },
  'third_party/node-addon-api': {
    'url': '{github_git}/nodejs/node-addon-api.git@4a3de56c3e4ed0031635a2f642b27efeeed00add',
    'condition': 'dawn_node',
  },
  'third_party/gpuweb': {
    'url': '{github_git}/gpuweb/gpuweb.git@17f7771857dd6cf0377a99e6a63a723b80485e50',
    'condition': 'dawn_node',
  },

  'tools/golang': {
    'packages': [{
      'package': 'infra/3pp/tools/go/${{platform}}',
      'version': Var('dawn_go_version'),
    }],
    'dep_type': 'cipd',
  },

  'tools/cmake': {
    'condition': 'dawn_node and (host_os == "mac" or host_os == "linux")',
    'packages': [{
      'package': 'infra/3pp/tools/cmake/${{platform}}',
      'version': Var('dawn_cmake_version'),
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

  # RBE dependencies
  'buildtools/reclient': {
    'packages': [
      {
        'package': Var('reclient_package') + '${{platform}}',
        'version': Var('reclient_version'),
      }
    ],
    'dep_type': 'cipd',
    'condition': 'dawn_standalone',
  },

  # Misc dependencies inherited from Tint
  'third_party/protobuf': {
    'url': '{chromium_git}/external/github.com/protocolbuffers/protobuf.git@2b673bbb57e34fe1bd4570f726fc86b769a3a3d2',
    'condition': 'dawn_standalone',
  },

  # Dependencies for PartitionAlloc.
  # Doc: https://docs.google.com/document/d/1wz45t0alQthsIU9P7_rQcfQyqnrBMXzrOjSzdQo-V-A
  'third_party/partition_alloc': {
    'url': '{chromium_git}/chromium/src/base/allocator/partition_allocator.git@6f90cb04abb81942abaab7b63d34c02882208172',
    'condition': 'dawn_standalone',
  },
}

hooks = [
  # Pull the compilers and system libraries for hermetic builds
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_linux and (checkout_x86 or checkout_x64)',
    'action': ['python3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_linux and checkout_x64',
    'action': ['python3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    # Update the Mac toolchain if possible, this makes builders use "hermetic XCode" which is
    # is more consistent (only changes when rolling build/) and is cached.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_mac',
    'action': ['python3', 'build/mac_toolchain.py'],
  },
  {
    # Case-insensitivity for the Win SDK. Must run before win_toolchain below.
    'name': 'ciopfs_linux',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "linux"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/ciopfs',
                '-s', 'build/ciopfs.sha1',
    ]
  },
  {
    # Update the Windows toolchain if necessary. Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win',
    'action': ['python3', 'build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python3', 'tools/clang/scripts/update.py'],
    'condition': 'dawn_standalone',
  },
  {
    # This is also supposed to support the same set of platforms as 'clang'
    # above. LLVM ToT support isn't provided at the moment.
    'name': 'clang_tidy',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_clang_tidy',
    'action': ['python3', 'tools/clang/scripts/update.py',
               '--package=clang-tidy'],
  },
  {
    'name': 'rust',
    'pattern': '.',
    'action': ['python3', 'tools/rust/update_rust.py'],
    'condition': 'dawn_standalone and checkout_rust',
  },
  {
    # Pull rc binaries using checked-in hashes.
    'name': 'rc_win',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'build/toolchain/win/rc/win/rc.exe.sha1',
    ],
  },
  {
    'name': 'rc_linux',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "linux"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'build/toolchain/win/rc/linux64/rc.sha1',
    ],
  },
  {
    'name': 'rc_mac',
    'pattern': '.',
    'condition': 'dawn_standalone and checkout_win and host_os == "mac"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'build/toolchain/win/rc/mac/rc.sha1',
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac_x64',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "mac" and host_cpu == "x64"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/mac/clang-format.x64.sha1',
                '-o', 'buildtools/mac/clang-format',
    ],
  },
  {
    'name': 'clang_format_mac_arm64',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "mac" and host_cpu == "arm64"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/mac/clang-format.arm64.sha1',
                '-o', 'buildtools/mac/clang-format',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'condition': 'dawn_standalone and host_os == "linux"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/linux64/clang-format.sha1',
    ],
  },
  # Update build/util/LASTCHANGE.
  {
    'name': 'lastchange',
    'pattern': '.',
    'condition': 'dawn_standalone',
    'action': ['python3', 'build/util/lastchange.py',
               '-o', 'build/util/LASTCHANGE'],
  },
  # TODO(https://crbug.com/1180257): Use CIPD for CMake on Windows.
  {
    'name': 'cmake_win32',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-tools',
                Var('dawn_cmake_win32_sha1'),
                '-o', 'tools/cmake-win32.zip'
    ],
  },
  {
    'name': 'cmake_win32_extract',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "win"',
    'action': [ 'python3',
                'scripts/extract.py',
                'tools/cmake-win32.zip',
                'tools/cmake-win32/',
    ],
  },

  # Node binaries, when dawn_node is enabled
  {
    'name': 'node_linux64',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "linux"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--extract',
                '--no_auth',
                '--bucket', 'chromium-nodejs/16.13.0',
                Var('node_linux_x64_sha'),
                '-o', 'third_party/node/node-linux-x64.tar.gz',
    ],
  },
  {
    'name': 'node_mac',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "mac"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--extract',
                '--no_auth',
                '--bucket', 'chromium-nodejs/16.13.0',
                Var('node_darwin_x64_sha'),
                '-o', 'third_party/node/node-darwin-x64.tar.gz',
    ],
  },
  {
    'name': 'node_mac_arm64',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "mac"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--extract',
                '--no_auth',
                '--bucket', 'chromium-nodejs/16.13.0',
                Var('node_darwin_arm64_sha'),
                '-o', 'third_party/node/node-darwin-arm64.tar.gz',
    ],
  },
  {
    'name': 'node_win',
    'pattern': '.',
    'condition': 'dawn_node and host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-nodejs/16.13.0',
                Var('node_win_x64_sha'),
                '-o', 'third_party/node/node.exe',
    ],
  },
 {
   # Download remote exec cfg files
   'name': 'fetch_reclient_cfgs',
   'pattern': '.',
   'condition': 'download_remoteexec_cfg and dawn_standalone',
   'action': ['python3',
              'buildtools/reclient_cfgs/fetch_reclient_cfgs.py',
              '--rbe_instance',
              Var('rbe_instance'),
              '--reproxy_cfg_template',
              'reproxy.cfg.template',
              '--rewrapper_cfg_project',
              Var('rewrapper_cfg_project'),
              '--quiet',
              ],
 },
]

recursedeps = [
  'third_party/vulkan-deps',
]
