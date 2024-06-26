SKIP: hlsl-dxc validation fails on debug builds due to https://github.com/microsoft/DirectXShaderCompiler/issues/5294

struct str {
  int i;
};

str func(inout str pointer) {
  return pointer;
}

static str P[4] = (str[4])0;

[numthreads(1, 1, 1)]
void main() {
  str r = func(P[2]);
  return;
}
