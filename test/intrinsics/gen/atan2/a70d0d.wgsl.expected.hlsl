SKIP: https://github.com/microsoft/DirectXShaderCompiler/issues/3824



Validation Failure:
void atan2_a70d0d() {
  float3 res = atan2(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  atan2_a70d0d();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  atan2_a70d0d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_a70d0d();
  return;
}

Internal Compiler error: cast<X>() argument of incompatible type!


Internal Compiler error: cast<X>() argument of incompatible type!


Internal Compiler error: cast<X>() argument of incompatible type!

