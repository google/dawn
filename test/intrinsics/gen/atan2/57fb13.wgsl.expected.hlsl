SKIP: https://github.com/microsoft/DirectXShaderCompiler/issues/3824



Validation Failure:
void atan2_57fb13() {
  float2 res = atan2(float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  atan2_57fb13();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  atan2_57fb13();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_57fb13();
  return;
}

Internal Compiler error: cast<X>() argument of incompatible type!


Internal Compiler error: cast<X>() argument of incompatible type!


Internal Compiler error: cast<X>() argument of incompatible type!

