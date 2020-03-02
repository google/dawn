use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',

  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'googletest_revision': '41b5f149ab306e96b5b2faf523505d75acffd98a',
  'spirv_headers_revision': '5dbc1c32182e17b8ab8e8158a802ecabaf35aad3',
  'spirv_tools_revision': 'fe10239f92f4539e9050da375dab095328fec196',
}

deps = {
  'third_party/cpplint': Var('google_git') + '/styleguide.git@' +
      Var('cpplint_revision'),

  'third_party/googletest': Var('google_git') + '/googletest.git@' +
      Var('googletest_revision'),

  'third_party/spirv-headers': Var('khronos_git') + '/SPIRV-Headers.git@' +
      Var('spirv_headers_revision'),

  'third_party/spirv-tools': Var('khronos_git') + '/SPIRV-Tools.git@' +
      Var('spirv_tools_revision'),
}
