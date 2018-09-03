vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'github_git': 'https://github.com',

  'dawn_root': '.',
  'dawn_standalone': True,
}

deps = {
  # Dependencies required to use GN/Clang in standalone
  '{dawn_root}/build': {
    'url': '{chromium_git}/chromium/src/build@ddcfe1a1c6428569cb8c900735be3567358bc6ee',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/buildtools': {
    'url': '{chromium_git}/chromium/buildtools@9a90d9aaadeb5e04327ed05775f45132e4b3523f',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/tools/clang': {
    'url': '{chromium_git}/chromium/src/tools/clang@d299f069f8d1dca337354ced634df7a78e4512f7',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/third_party/binutils': {
    'url': '{chromium_git}/chromium/src/third_party/binutils@4110e09197116a9c5dedd4c827bbe95c224f87ac',
    'condition': 'dawn_standalone',
  },

  # Testing, GTest and GMock
  '{dawn_root}/testing': {
    'url': '{chromium_git}/chromium/src/testing@b07830f6905ce9e33034ad14820bc0a58b6e9e41',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/third_party/googletest': {
    'url': '{chromium_git}/external/github.com/google/googletest@98a0d007d7092b72eea0e501bb9ad17908a1a036',
    'condition': 'dawn_standalone',
  },

  # Jinja2 and MarkupSafe for the code generator
  '{dawn_root}/third_party/jinja2': {
    'url': '{chromium_git}/chromium/src/third_party/jinja2@b41863e42637544c2941b574c7877d3e1f663e25',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/third_party/markupsafe': {
    'url': '{chromium_git}/chromium/src/third_party/markupsafe@8f45f5cfa0009d2a70589bcda0349b8cb2b72783',
    'condition': 'dawn_standalone',
  },

  # SPIRV-Cross
  '{dawn_root}/third_party/spirv-cross': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Cross@a7697446b12666da353bb2bdafa792d988fb268c',
    'condition': 'dawn_standalone',
  },

  # SPIRV compiler dependencies: SPIRV-Tools, SPIRV-headers, glslang and shaderc
  '{dawn_root}/third_party/SPIRV-Tools': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Tools@eda2cfbe128e5b71e9a0131f816ade5186ad6420',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/third_party/spirv-headers': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Headers@ff684ffc6a35d2a58f0f63108877d0064ea33feb',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/third_party/glslang': {
    'url': '{chromium_git}/external/github.com/google/glslang@29619b2312f7bc862221749f3f4d37c3e6a0dee2',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/third_party/shaderc': {
    'url': '{chromium_git}/external/github.com/google/shaderc@30af9f9899aefd018669e81a5b8e605d14d40431',
    'condition': 'dawn_standalone',
  },

  # GLFW for tests and samples
  '{dawn_root}/third_party/glfw': {
    'url': '{chromium_git}/external/github.com/glfw/glfw@096efdf798896cff80a0b2db08d7398b703406fe',
    'condition': 'dawn_standalone',
  },

  # Dependencies for samples: stb and GLM
  '{dawn_root}/third_party/stb': {
    'url': '{github_git}/nothings/stb.git@c7110588a4d24c4bb5155c184fbb77dd90b3116e',
    'condition': 'dawn_standalone',
  },
  '{dawn_root}/third_party/glm': {
    'url': '{github_git}/g-truc/glm.git@06f084063fd6d9aa2ef6904517650700ae47b63d',
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
    # Update the Windows toolchain if necessary. Must run before 'clang' below.
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
  # Pull binutils for linux hermetic builds
  {
    'name': 'binutils',
    'pattern': 'src/third_party/binutils',
    'condition': 'host_os == "linux"',
    'action': [
        'python',
        '{dawn_root}/third_party/binutils/download.py',
    ],
  },
]

recursedeps = [
  # buildtools provides clang_format, libc++, and libc++abi
  '{dawn_root}/buildtools',
]
