SKIP: https://github.com/microsoft/DirectXShaderCompiler/issues/3823



Validation Failure:
void isNan_1280ab() {
  bool3 res = isnan(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  isNan_1280ab();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  isNan_1280ab();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNan_1280ab();
  return;
}

Internal Compiler error: cast<X>() argument of incompatible type!


Internal Compiler error: cast<X>() argument of incompatible type!


Internal Compiler error: cast<X>() argument of incompatible type!

