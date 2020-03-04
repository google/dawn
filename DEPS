use_relative_paths = True

vars = {
  'chromium_git':  'https://chromium.googlesource.com',
  'github': '/external/github.com',

  'cpplint_revision': '305ac8725a166ed42e3f5dd3f80d6de2cf840ef1',
  'googletest_revision': '703ca235f0a83aeebf2dfe2cc56a7eac362cf078',
  'spirv_headers_revision': '0a7fc45259910f07f00c5a3fa10be5678bee1f83',
  'spirv_tools_revision': 'e1688b60caf77e7efd9e440e57cca429ca7c5a1e',
}

deps = {
  'third_party/cpplint': Var('chromium_git') + Var('github') +
      '/google/styleguide.git@' + Var('cpplint_revision'),

  'third_party/googletest': Var('chromium_git') + '/chromium/src/third_party' +
      '/googletest.git@' + Var('googletest_revision'),

  'third_party/spirv-headers': Var('chromium_git') + Var('github') +
      '/KhronosGroup/SPIRV-Headers.git@' + Var('spirv_headers_revision'),

  'third_party/spirv-tools': Var('chromium_git') + Var('github') +
      '/KhronosGroup//SPIRV-Tools.git@' + Var('spirv_tools_revision'),
}
