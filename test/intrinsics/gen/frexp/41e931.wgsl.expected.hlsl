float tint_frexp(float param_0, inout int param_1) {
  float float_exp;
  float significand = frexp(param_0, float_exp);
  param_1 = int(float_exp);
  return significand;
}

void frexp_41e931() {
  int arg_1 = 0;
  float res = tint_frexp(1.0f, arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  frexp_41e931();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_41e931();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_41e931();
  return;
}
