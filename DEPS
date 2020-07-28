use_relative_paths = True

gclient_gn_args_file = 'build/config/gclient_args.gni'
gclient_gn_args = [
  'mac_xcode_version',
]

vars = {
  # This can be overridden, e.g. with custom_vars, to download a nonstandard
  # Xcode version in build/mac_toolchain.py
  # instead of downloading the prebuilt pinned revision.
  'mac_xcode_version': 'default',

  'chromium_git':  'https://chromium.googlesource.com',
  'github': '/external/github.com',

  'build_revision': '9b01e295f87e0cbdef297148e2b14efdd5f9d699',
  'buildtools_revision': 'eb3987ec709b39469423100c1e77f0446890e059',
  'clang_revision': '6412135b3979b680c20cf007ab242d968025fc3e',
  'cpplint_revision': '305ac8725a166ed42e3f5dd3f80d6de2cf840ef1',
  'googletest_revision': 'a781fe29bcf73003559a3583167fe3d647518464',
  'gpuweb_cts_revision': '40e337a38784ad72fa5c7b9afd1b9c358a9e0f1a',
  'spirv_headers_revision': '308bd07424350a6000f35a77b5f85cd4f3da319e',
  'spirv_tools_revision': '717e7877cac15d393fd3bb1bd872679de8b59add',
  'testing_revision': 'cadd4e1eb3a45f562cc7eb5cd646c9b6f91c87dc',
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
