vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'github_git': 'https://github.com',

  'dawn_root': '.',
  'dawn_standalone': True,
}

deps = {
  # Dependencies required to use GN/Clang in standalone
  '{dawn_root}/build': {
    'url': '{chromium_git}/chromium/src/build.git@b944b99e72923c5a6699235ed858e725db21f81f',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/buildtools': {
    'url': '{chromium_git}/chromium/buildtools.git@94288c26d2ffe3aec9848c147839afee597acefd',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/tools/clang': {
    'url': '{chromium_git}/chromium/src/tools/clang.git@c893c7eec4706f8c7fc244ee254b1dadd8f8d158',
    'condition': 'dawn_standalone',
  },

  # Testing, GTest and GMock
  '{dawn_root}/testing': {
    'url': '{chromium_git}/chromium/src/testing@4d706fd80be9e8989aec5235540e7b46d0672826',
    'condition': 'dawn_standalone',
  },
  'third_party/googletest': {
    'url': '{github_git}/google/googletest.git@98a0d007d7092b72eea0e501bb9ad17908a1a036',
    'condition': 'dawn_standalone',
  },

  # SPIRV-Cross
  '{dawn_root}/third_party/spirv-cross': {
    'url': '{github_git}/Kangz/SPIRV-Cross.git@694cad533296df02b4562f4a5a20cba1d1a9dbaf',
    'condition': 'dawn_standalone',
  },

  # SPIRV compiler dependencies: SPIRV-Tools, SPIRV-headers, glslang and shaderc
  '{dawn_root}/third_party/SPIRV-Tools': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Tools.git@eda2cfbe128e5b71e9a0131f816ade5186ad6420',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/third_party/spirv-headers': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Headers.git@ff684ffc6a35d2a58f0f63108877d0064ea33feb',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/third_party/glslang': {
    'url': '{chromium_git}/external/github.com/google/glslang.git@29619b2312f7bc862221749f3f4d37c3e6a0dee2',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/third_party/shaderc': {
    'url': '{chromium_git}/external/github.com/google/shaderc.git@30af9f9899aefd018669e81a5b8e605d14d40431',
    'condition': 'dawn_standalone',
  },

}

hooks = [
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'condition': 'host_os == "win" and dawn_standalone',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', '{dawn_root}/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'name': 'clang_format_mac',
    'pattern': '.',
    'condition': 'host_os == "mac" and dawn_standalone',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', '{dawn_root}/buildtools/mac/clang-format.sha1',
    ],
  },
  {
    'name': 'clang_format_linux',
    'pattern': '.',
    'condition': 'host_os == "linux" and dawn_standalone',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', '{dawn_root}/buildtools/linux64/clang-format.sha1',
    ],
  },

  # Pull GN binaries using checked-in hashes.
  {
    'name': 'gn_win',
    'pattern': '.',
    'condition': 'host_os == "win" and dawn_standalone',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', '{dawn_root}/buildtools/win/gn.exe.sha1',
    ],
  },
  {
    'name': 'gn_mac',
    'pattern': '.',
    'condition': 'host_os == "mac" and dawn_standalone',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=darwin',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', '{dawn_root}/buildtools/mac/gn.sha1',
    ],
  },
  {
    'name': 'gn_linux64',
    'pattern': '.',
    'condition': 'host_os == "linux" and dawn_standalone',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=linux*',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', '{dawn_root}/buildtools/linux64/gn.sha1',
    ],
  },

  # Pull the compilers and system libraries for hermetic builds
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'checkout_linux and ((checkout_x86 or checkout_x64) and dawn_standalone)',
    'action': ['python', '{dawn_root}/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'checkout_linux and (checkout_x64 and dawn_standalone)',
    'action': ['python', '{dawn_root}/build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    # Update the Windows toolchain if necessary.  Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'checkout_win and dawn_standalone',
    'action': ['python', '{dawn_root}/build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', '{dawn_root}/tools/clang/scripts/update.py'],
    'condition': 'dawn_standalone',
  },
  {
    # Pull rc binaries using checked-in hashes.
    'name': 'rc_win',
    'pattern': '.',
    'condition': 'checkout_win and (host_os == "win" and dawn_standalone)',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--no_auth',
                '--bucket', 'chromium-browser-clang/rc',
                '-s', '{dawn_root}/build/toolchain/win/rc/win/rc.exe.sha1',
    ],
  },
]
