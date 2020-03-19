use_relative_paths = True

vars = {
  'chromium_git':  'https://chromium.googlesource.com',
  'github': '/external/github.com',

  'cpplint_revision': '305ac8725a166ed42e3f5dd3f80d6de2cf840ef1',
  'googletest_revision': '482ac6ee63429af2aa9c44f4e6427873fb68fb1f',
  'spirv_headers_revision': 'f8bf11a0253a32375c32cad92c841237b96696c0',
  'spirv_tools_revision': '60104cd97446877dad8ed1010a635218937a2f18',
}

deps = {
  'third_party/cpplint': Var('chromium_git') + Var('github') +
      '/google/styleguide.git@' + Var('cpplint_revision'),

  'third_party/googletest': Var('chromium_git') + Var('github') +
      '/google/googletest.git@' + Var('googletest_revision'),

  'third_party/spirv-headers': Var('chromium_git') + Var('github') +
      '/KhronosGroup/SPIRV-Headers.git@' + Var('spirv_headers_revision'),

  'third_party/spirv-tools': Var('chromium_git') + Var('github') +
      '/KhronosGroup//SPIRV-Tools.git@' + Var('spirv_tools_revision'),
}
