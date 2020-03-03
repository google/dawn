use_relative_paths = True

vars = {
  'google_git':  'https://github.com/google',
  'khronos_git': 'https://github.com/KhronosGroup',

  'cpplint_revision': '26470f9ccb354ff2f6d098f831271a1833701b28',
  'googletest_revision': 'e588eb1ff9ff6598666279b737b27f983156ad85',
  'spirv_headers_revision': '0a7fc45259910f07f00c5a3fa10be5678bee1f83',
  'spirv_tools_revision': 'e1688b60caf77e7efd9e440e57cca429ca7c5a1e',
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
