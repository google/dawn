float4 tint_frexp(float4 param_0, inout int4 param_1) {
  float4 float_exp;
  float4 significand = frexp(param_0, float_exp);
  param_1 = int4(float_exp);
  return significand;
}

void frexp_19ab15() {
  int4 arg_1 = int4(0, 0, 0, 0);
  float4 res = tint_frexp(float4(0.0f, 0.0f, 0.0f, 0.0f), arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  frexp_19ab15();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_19ab15();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_19ab15();
  return;
}
