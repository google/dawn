float2 tint_frexp(float2 param_0, inout int2 param_1) {
  float2 float_exp;
  float2 significand = frexp(param_0, float_exp);
  param_1 = int2(float_exp);
  return significand;
}

static int2 arg_1 = int2(0, 0);

void frexp_c084e3() {
  float2 res = tint_frexp(float2(0.0f, 0.0f), arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  frexp_c084e3();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_c084e3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_c084e3();
  return;
}
