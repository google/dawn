use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

gclient_gn_args = [
  'generate_location_tags',
]

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'dawn_git': 'https://dawn.googlesource.com',
  'github_git': 'https://github.com',
  'swiftshader_git': 'https://swiftshader.googlesource.com',

  'dawn_standalone': True,
  'dawn_node': False, # Also fetches dependencies required for building NodeJS bindings.
  'dawn_cmake_version': 'version:3.13.5',
  'dawn_cmake_win32_sha1': 'b106d66bcdc8a71ea2cdf5446091327bfdb1bcd7',
  'dawn_gn_version': 'git_revision:bd99dbf98cbdefe18a4128189665c5761263bcfb',
  'dawn_go_version': 'version:1.16',

  'node_darwin_arm64_sha': '31859fc1fa0994a95f44f09c367d6ff63607cfde',
  'node_darwin_x64_sha': '16dfd094763b71988933a31735f9dea966f9abd6',
  'node_linux_x64_sha': 'ab9544e24e752d3d17f335fb7b2055062e582d11',
  'node_win_x64_sha': '5ef847033c517c499f56f9d136d159b663bab717',

  # GN variable required by //testing that will be output in the gclient_args.gni
  'generate_location_tags': False,
}

deps = {
  # Dependencies required to use GN/Clang in standalone
  'build': {
    'url': '{chromium_git}/chromium/src/build@87b04ad66530e4a571cef36d6e71ef737d23a887',
    'condition': 'dawn_standalone',
  },
  'buildtools': {
    'url': '{chromium_git}/chromium/src/buildtools@f0d740e4e2f803e39dfd5d8d11f7d87bdf489514',
    'condition': 'dawn_standalone',
  },
  'buildtools/clang_format/script': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@99803d74e35962f63a775f29477882afd4d57d94',
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

  'buildtools/third_party/libc++/trunk': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxx.git@79a2e924d96e2fc1e4b937c42efd08898fa472d7',
    'condition': 'dawn_standalone',
  },

  'buildtools/third_party/libc++abi/trunk': {
    'url': '{chromium_git}/external/github.com/llvm/llvm-project/libcxxabi.git@edde7bbc4049ae4a32257d9f16451312c763c601',
    'condition': 'dawn_standalone',
  },

  'tools/clang': {
    'url': '{chromium_git}/chromium/src/tools/clang@3c4a622d9f0b0ce5ec2a438189d46c695216b324',
    'condition': 'dawn_standalone',
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
    'url': '{chromium_git}/chromium/src/testing@d485ae97b7900c1fb7edfbe2901ae5adcb120865',
    'condition': 'dawn_standalone',
  },
  'third_party/googletest': {
    'url': '{chromium_git}/external/github.com/google/googletest@6b74da4757a549563d7c37c8fae3e704662a043b',
    'condition': 'dawn_standalone',
  },
  # This is a dependency of //testing
  'third_party/catapult': {
    'url': '{chromium_git}/catapult.git@fa35beefb3429605035f98211ddb8750dee6a13d',
    'condition': 'dawn_standalone',
  },

  # Jinja2 and MarkupSafe for the code generator
  'third_party/jinja2': {
    'url': '{chromium_git}/chromium/src/third_party/jinja2@ee69aa00ee8536f61db6a451f3858745cf587de6',
    'condition': 'dawn_standalone',
  },
  'third_party/markupsafe': {
    'url': '{chromium_git}/chromium/src/third_party/markupsafe@0944e71f4b2cb9a871bcbe353f95e889b64a611a',
    'condition': 'dawn_standalone',
  },

  # GLFW for tests and samples
  'third_party/glfw': {
    'url': '{chromium_git}/external/github.com/glfw/glfw@94773111300fee0453844a4c9407af7e880b4df8',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan_memory_allocator': {
    'url': '{chromium_git}/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@5e49f57a6e71a026a54eb42e366de09a4142d24e',
    'condition': 'dawn_standalone',
  },

  'third_party/angle': {
    'url': '{chromium_git}/angle/angle@2ce60b52a23954985a76dd8117ed8da7b0392a51',
    'condition': 'dawn_standalone',
  },

  'third_party/swiftshader': {
    'url': '{swiftshader_git}/SwiftShader@62c1af08ea4d205e87536b50dfa775cc5bbe6a0b',
    'condition': 'dawn_standalone',
  },

  'third_party/vulkan-deps': {
    'url': '{chromium_git}/vulkan-deps@4957ae73444562b50903e24e7198cb9ef2765388',
    'condition': 'dawn_standalone',
  },

  'third_party/zlib': {
    'url': '{chromium_git}/chromium/src/third_party/zlib@c29ee8c9c3824ca013479bf8115035527967fe02',
    'condition': 'dawn_standalone',
  },

  'third_party/abseil-cpp': {
    'url': '{chromium_git}/chromium/src/third_party/abseil-cpp@789af048b388657987c59d4da406859034fe310f',
    'condition': 'dawn_standalone',
  },

  # WebGPU CTS - not used directly by Dawn, only transitively by Chromium.
  'third_party/webgpu-cts': {
    'url': '{chromium_git}/external/github.com/gpuweb/cts@ecf4804228d1473df8596c3c87e5b7e32649e2de',
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
    'url': '{github_git}/gpuweb/gpuweb.git@16df823c91c9045b7cdf9bd0f2c0ef6d43ac95e7',
    'condition': 'dawn_node',
  },
  'third_party/gpuweb-cts': {
    'url': '{chromium_git}/external/github.com/gpuweb/cts@ecf4804228d1473df8596c3c87e5b7e32649e2de',
    'condition': 'dawn_standalone',
  },

  'tools/golang': {
    'condition': 'dawn_node',
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

  # Misc dependencies inherited from Tint
  'third_party/benchmark': {
    'url': '{chromium_git}/external/github.com/google/benchmark.git@e991355c02b93fe17713efe04cbc2e278e00fdbd',
    'condition': 'dawn_standalone',
  },
  'third_party/protobuf': {
    'url': '{chromium_git}/external/github.com/protocolbuffers/protobuf.git@fde7cf7358ec7cd69e8db9be4f1fa6a5c431386a',
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

]

recursedeps = [
  'third_party/vulkan-deps',
]
