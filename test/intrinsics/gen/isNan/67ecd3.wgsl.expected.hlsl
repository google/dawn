SKIP: https://github.com/microsoft/DirectXShaderCompiler/issues/3823



Validation Failure:
void isNan_67ecd3() {
  bool2 res = isnan(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  isNan_67ecd3();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  isNan_67ecd3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isNan_67ecd3();
  return;
}

Internal Compiler error: cast<X>() argument of incompatible type!


Internal Compiler error: cast<X>() argument of incompatible type!


Internal Compiler error: cast<X>() argument of incompatible type!

