use_relative_paths = True

vars = {
  'chromium_git':  'https://chromium.googlesource.com',
  'github': '/external/github.com',

  'binutils_revision': '2566778206fdacdff455b2d96df3d52a18d439d4',
  'build_revision': '1a541c15828e268c005eb956adc09816af2b01a5',
  'buildtools_revision': '6b951673d17662ea2510995700922657d30f3542',
  'clang_revision': 'b49c12a8f3fb25181855d0ea641bbcc2b2e95213',
  'cpplint_revision': '305ac8725a166ed42e3f5dd3f80d6de2cf840ef1',
  'googletest_revision': '356f2d264a485db2fcc50ec1c672e0d37b6cb39b',
  'spirv_headers_revision': '11d7637e7a43cd88cfd4e42c99581dcb682936aa',
  'spirv_tools_revision': 'bd2a9ea85210d3bb474bc5adb9ff4b0bb536b4fc',
  'testing_revision': 'a711b1132571869bdd7084c86083698671b290a6',
}

deps = {
  'third_party/cpplint': Var('chromium_git') + Var('github') +
      '/google/styleguide.git@' + Var('cpplint_revision'),

  'third_party/spirv-headers': Var('chromium_git') + Var('github') +
      '/KhronosGroup/SPIRV-Headers.git@' + Var('spirv_headers_revision'),

  'third_party/spirv-tools': Var('chromium_git') + Var('github') +
      '/KhronosGroup//SPIRV-Tools.git@' + Var('spirv_tools_revision'),

  # Dependencies required to use GN/Clang in standalone
  'build': Var('chromium_git') + '/chromium/src/build@' +
      Var('build_revision'),

  'buildtools': Var('chromium_git') + '/chromium/src/buildtools@' +
      Var('buildtools_revision'),

  'tools/clang': Var('chromium_git') + '/chromium/src/tools/clang@' +
      Var('clang_revision'),

  'third_party/binutils': Var('chromium_git') +
      '/chromium/src/third_party/binutils@' + Var('binutils_revision'),

  # Dependencies required for testing
  'testing': Var('chromium_git') + '/chromium/src/testing@' +
      Var('testing_revision'),

  'third_party/googletest': Var('chromium_git') + Var('github') +
      '/google/googletest.git@' + Var('googletest_revision'),
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
    'action': ['python', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'checkout_linux and (checkout_x64)',
    'action': ['python', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    # Update the Windows toolchain if necessary. Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'checkout_win',
    'action': ['python', 'build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'tools/clang/scripts/update.py'],
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
  # Pull binutils for linux hermetic builds
  {
    'name': 'binutils',
    'pattern': 'src/third_party/binutils',
    'condition': 'host_os == "linux"',
    'action': [
        'python',
        'third_party/binutils/download.py',
    ],
  },
  # Update build/util/LASTCHANGE.
  {
    'name': 'lastchange',
    'pattern': '.',
    'action': ['python', 'build/util/lastchange.py',
               '-o', 'build/util/LASTCHANGE'],
  },
]

recursedeps = [
  # buildtools provides clang_format, libc++, and libc++abi
  'buildtools',
]
