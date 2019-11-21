use_relative_paths = True
use_relative_hooks = True

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'dawn_git': 'https://dawn.googlesource.com',
  'github_git': 'https://github.com',

  'dawn_standalone': True,
}

deps = {
  # Dependencies required to use GN/Clang in standalone
  'build': {
    'url': '{chromium_git}/chromium/src/build@f3d0ca5f46b7b190dbbdc6be508ca11dd5c54302',
    'condition': 'dawn_standalone',
  },
  'buildtools': {
    'url': '{chromium_git}/chromium/src/buildtools@74cfb57006f83cfe050817526db359d5c8a11628',
    'condition': 'dawn_standalone',
  },
  'tools/clang': {
    'url': '{chromium_git}/chromium/src/tools/clang@3605577b67603ec5776afcfada9e0ff4ea05cf0e',
    'condition': 'dawn_standalone',
  },
  'third_party/binutils': {
    'url': '{chromium_git}/chromium/src/third_party/binutils@01aa7745b0bab64ae22600f09fd6483c60f22ebf',
    'condition': 'dawn_standalone',
  },

  # Testing, GTest and GMock
  'testing': {
    'url': '{chromium_git}/chromium/src/testing@2ffbbb3c8e33d51ddb3cc6b8cd10588302c33628',
    'condition': 'dawn_standalone',
  },
  'third_party/googletest': {
    'url': '{chromium_git}/external/github.com/google/googletest@5ec7f0c4a113e2f18ac2c6cc7df51ad6afc24081',
    'condition': 'dawn_standalone',
  },

  # Jinja2 and MarkupSafe for the code generator
  'third_party/jinja2': {
    'url': '{chromium_git}/chromium/src/third_party/jinja2@b41863e42637544c2941b574c7877d3e1f663e25',
    'condition': 'dawn_standalone',
  },
  'third_party/markupsafe': {
    'url': '{chromium_git}/chromium/src/third_party/markupsafe@8f45f5cfa0009d2a70589bcda0349b8cb2b72783',
    'condition': 'dawn_standalone',
  },

  # SPIRV-Cross
  'third_party/spirv-cross': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Cross@fd5aa3ad51ece55a1b51fe6bfb271db6844ae291',
    'condition': 'dawn_standalone',
  },

  # SPIRV compiler dependencies: SPIRV-Tools, SPIRV-headers, glslang and shaderc
  'third_party/SPIRV-Tools': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Tools@2ee9aaa288d91fbaec5bc51473169c079c764240',
    'condition': 'dawn_standalone',
  },
  'third_party/spirv-headers': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/SPIRV-Headers@204cd131c42b90d129073719f2766293ce35c081',
    'condition': 'dawn_standalone',
  },
  'third_party/glslang': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/glslang@f4d4668529f1dad0e475295456b601353fe7cf33',
    'condition': 'dawn_standalone',
  },
  'third_party/shaderc': {
    'url': '{chromium_git}/external/github.com/google/shaderc@eca11511a0c09dd599837862c96658a91e927a3a',
    'condition': 'dawn_standalone',
  },

  # jsoncpp for perf tests trace events
  'third_party/jsoncpp': {
    'url': '{chromium_git}/chromium/src/third_party/jsoncpp@571788934b5ee8643d53e5d054534abbe6006168',
    'condition': 'dawn_standalone',
  },

  'third_party/jsoncpp/source': {
    'url' : '{chromium_git}/external/github.com/open-source-parsers/jsoncpp@645250b6690785be60ab6780ce4b58698d884d11',
    'condition': 'dawn_standalone',
  },

  # GLFW for tests and samples
  'third_party/glfw': {
    'url': '{chromium_git}/external/github.com/glfw/glfw@2de2589f910b1a85905f425be4d32f33cec092df',
    'condition': 'dawn_standalone',
  },

  # Dependencies for samples: GLM
  'third_party/glm': {
    'url': '{github_git}/g-truc/glm.git@06f084063fd6d9aa2ef6904517650700ae47b63d',
    'condition': 'dawn_standalone',
  },

  # Our own pre-compiled Linux clang-format 7.0 for presubmit
  'third_party/clang-format': {
    'url': '{dawn_git}/clang-format@2451c56cd368676cdb230fd5ad11731ab859f1a3',
    'condition': 'dawn_standalone and checkout_linux',
  },

  # Khronos Vulkan-Headers
  'third_party/vulkan-headers': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-Headers@5b44df19e040fca0048ab30c553a8c2d2cb9623e',
    'condition': 'dawn_standalone',
  },

  # Khronos Vulkan-ValidationLayers
  'third_party/vulkan-validation-layers': {
    'url': '{chromium_git}/external/github.com/KhronosGroup/Vulkan-ValidationLayers@9fba37afae13a11bd49ae942bf82e5bf1098e381',
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
                '-s', 'buildtools/win/clang-format.exe.sha1',
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
                '-s', 'buildtools/mac/clang-format.sha1',
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
                '-s', 'buildtools/linux64/clang-format.sha1',
    ],
  },

  # Pull the compilers and system libraries for hermetic builds
  {
    'name': 'sysroot_x86',
    'pattern': '.',
    'condition': 'checkout_linux and ((checkout_x86 or checkout_x64) and dawn_standalone)',
    'action': ['python', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x86'],
  },
  {
    'name': 'sysroot_x64',
    'pattern': '.',
    'condition': 'checkout_linux and (checkout_x64 and dawn_standalone)',
    'action': ['python', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=x64'],
  },
  {
    # Update the Windows toolchain if necessary. Must run before 'clang' below.
    'name': 'win_toolchain',
    'pattern': '.',
    'condition': 'checkout_win and dawn_standalone',
    'action': ['python', 'build/vs_toolchain.py', 'update', '--force'],
  },
  {
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'tools/clang/scripts/update.py'],
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
