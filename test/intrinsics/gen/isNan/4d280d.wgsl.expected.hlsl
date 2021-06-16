SKIP: https://github.com/microsoft/DirectXShaderCompiler/issues/3823



Validation Failure:
void isNan_4d280d() {
  bool4 res = isnan(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  isNan_4d280d();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  isNan_4d280d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNan_4d280d();
  return;
}

Internal Compiler error: cast<X>() argument of incompatible type!


Internal Compiler error: cast<X>() argument of incompatible type!


Internal Compiler error: cast<X>() argument of incompatible type!

