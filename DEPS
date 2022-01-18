use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

gclient_gn_args = [
  'generate_location_tags',
]

vars = {
  'chromium_git':  'https://chromium.googlesource.com',
  'github': '/external/github.com',

  'benchmark_revision': 'e991355c02b93fe17713efe04cbc2e278e00fdbd',
  'build_revision': '555c8b467c21e2c4b22d00e87e3faa0431df9ac2',
  'buildtools_revision': 'f78b4b9f33bd8ef9944d5ce643daff1c31880189',
  'catapult_revision': 'fa35beefb3429605035f98211ddb8750dee6a13d',
  'clang_format_revision': '2271e89c145a5e27d6c110b6a1113c057a8301a3',
  'clang_revision': '8b7330592cb85ba09505a6be7bacabd0ad6160a3',
  'glslang_revision': '4b7b86d568b40f4b076259dc2fc4cdd006340f34',
  'googletest_revision': '6b74da4757a549563d7c37c8fae3e704662a043b',
  'gpuweb_cts_revision': 'b0291fd966b55a5efc496772555b94842bde1085',
  'libcxx_revision': '79a2e924d96e2fc1e4b937c42efd08898fa472d7',
  'libcxxabi_revision': '2715a6c0de8dac4c7674934a6b3d30ba0c685271',
  'protobuf_revision': 'fde7cf7358ec7cd69e8db9be4f1fa6a5c431386a',
  'spirv_headers_revision': 'eddd4dfc930f1374a70797460240a501c7d333f7',
  'spirv_tools_revision': 'df2aad68b98279412494a6d449bd71b6756e699b',
  'testing_revision': 'd485ae97b7900c1fb7edfbe2901ae5adcb120865',
  'tint_gn_revision': 'git_revision:281ba2c91861b10fec7407c4b6172ec3d4661243',

  # We don't use location metadata in our test isolates.
  'generate_location_tags': False,
}

deps = {
  'third_party/gpuweb-cts': Var('chromium_git') + Var('github') +
      '/gpuweb/cts.git@' + Var('gpuweb_cts_revision'),

  'third_party/spirv-headers': Var('chromium_git') + Var('github') +
      '/KhronosGroup/SPIRV-Headers.git@' + Var('spirv_headers_revision'),

  'third_party/spirv-tools': Var('chromium_git') + Var('github') +
      '/KhronosGroup//SPIRV-Tools.git@' + Var('spirv_tools_revision'),

  'third_party/glslang': Var('chromium_git') + Var('github') +
      '/KhronosGroup/glslang.git@' + Var('glslang_revision'),

  # Dependencies required to use GN/Clang in standalone
  'build': Var('chromium_git') + '/chromium/src/build@' +
      Var('build_revision'),

  'buildtools': Var('chromium_git') + '/chromium/src/buildtools@' +
      Var('buildtools_revision'),

  'tools/clang': Var('chromium_git') + '/chromium/src/tools/clang@' +
      Var('clang_revision'),

  'buildtools/clang_format/script': {
    'url': Var('chromium_git') +
      '/external/github.com/llvm/llvm-project/clang/tools/clang-format.git@' +
        Var('clang_format_revision'),
  },

  'buildtools/linux64': {
    'packages': [{
      'package': 'gn/gn/linux-amd64',
      'version': Var('tint_gn_revision'),
    }],
    'dep_type': 'cipd',
    'condition': 'host_os == "linux"',
  },
  'buildtools/mac': {
    'packages': [{
      'package': 'gn/gn/mac-${{arch}}',
      'version': Var('tint_gn_revision'),
    }],
    'dep_type': 'cipd',
    'condition': 'host_os == "mac"',
  },
  'buildtools/win': {
    'packages': [{
      'package': 'gn/gn/windows-amd64',
      'version': Var('tint_gn_revision'),
    }],
    'dep_type': 'cipd',
    'condition': 'host_os == "win"',
  },

  'buildtools/third_party/libc++/trunk':
    Var('chromium_git') +
      '/external/github.com/llvm/llvm-project/libcxx.git@' +
        Var('libcxx_revision'),

  'buildtools/third_party/libc++abi/trunk':
    Var('chromium_git') +
      '/external/github.com/llvm/llvm-project/libcxxabi.git@' +
        Var('libcxxabi_revision'),

  # Dependencies required for testing
  'testing': Var('chromium_git') + '/chromium/src/testing@' +
      Var('testing_revision'),

  'third_party/catapult': Var('chromium_git') + '/catapult.git@' +
      Var('catapult_revision'),

  'third_party/benchmark': Var('chromium_git') + Var('github') +
      '/google/benchmark.git@' + Var('benchmark_revision'),

  'third_party/googletest': Var('chromium_git') + Var('github') +
      '/google/googletest.git@' + Var('googletest_revision'),

  'third_party/protobuf': Var('chromium_git') + Var('github') +
      '/protocolbuffers/protobuf.git@' + Var('protobuf_revision'),
}

hooks = [
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'condition': 'host_os == "win"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'condition': 'host_os == "mac"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'condition': 'host_os == "linux"',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'buildtools/linux64/clang-format.sha1',
    ],
  },

  # Pull the compilers and system libraries for hermetic builds
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'checkout_linux and ((checkout_x86 or checkout_x64))',
    'action': ['python3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'checkout_linux and (checkout_x64)',
    'action': ['python3', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    # Update the Mac toolchain if necessary.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'checkout_mac',
    'action': ['python3', 'build/mac_toolchain.py'],
  },
  {
    # Update the Windows toolchain if necessary. Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'checkout_win',
    'action': ['python3', 'build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python3', 'tools/clang/scripts/update.py'],
  },
  {
    # Pull rc binaries using checked-in hashes.
    'name': 'rc_win',
    'pattern': '.',
    'condition': 'checkout_win and (host_os == "win")',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', 'build/toolchain/win/rc/win/rc.exe.sha1',
    ],
  },
  # Update build/util/LASTCHANGE.
  {
    'name': 'lastchange',
    'pattern': '.',
    'action': ['python3', 'build/util/lastchange.py',
               '-o', 'build/util/LASTCHANGE'],
  },
]

recursedeps = [
  # buildtools provides clang_format, libc++, and libc++abi
  'buildtools',
]
