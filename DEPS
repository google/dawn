use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'

vars = {
  'chromium_git':  'https://chromium.googlesource.com',
  'github': '/external/github.com',

  'build_revision': 'b1d8cdddd35b2ccf16a4945748b1661b19201785',
  'buildtools_revision': 'ff93f3ea1a7f033d3caf8f60ec1937cc71351419',
  'clang_revision': 'fcef86e30a0ab061b982b5c9d91bb060df8f5269',
  'cpplint_revision': '305ac8725a166ed42e3f5dd3f80d6de2cf840ef1',
  'googletest_revision': 'b1fbd33c06cdb0024c67733c6fdec2009d17b384',
  'gpuweb_cts_revision': 'abe6e1dd262d96749966027fdf20eb4a34710ba1',
  'spirv_headers_revision': '104ecc356c1bea4476320faca64440cd1df655a3',
  'spirv_tools_revision': '2c458414c0851b9a0d1b0493857b56a1089847ac',
  'testing_revision': '2691851e49de541c3fe42fa8692ddcdee938162f',
}

deps = {
  'third_party/cpplint': Var('chromium_git') + Var('github') +
      '/google/styleguide.git@' + Var('cpplint_revision'),

  'third_party/gpuweb-cts': Var('chromium_git') + Var('github') +
      '/gpuweb/cts.git@' + Var('gpuweb_cts_revision'),

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
    # Update the Mac toolchain if necessary.
    'name': 'mac_toolchain',
    'pattern': '.',
    'condition': 'checkout_mac',
    'action': ['python', 'build/mac_toolchain.py'],
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
