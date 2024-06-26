SKIP: hlsl-dxc validation fails on debug builds due to https://github.com/microsoft/DirectXShaderCompiler/issues/5294

struct S {
  int m;
};

static S A[4] = (S[4])0;

void f() {
  S tint_symbol = {1};
  A[0] = tint_symbol;
  return;
}
