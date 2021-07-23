struct frexp_result_vec2 {
  float2 sig;
  int2 exp;
};
frexp_result_vec2 tint_frexp(float2 param_0) {
  float2 exp;
  float2 sig = frexp(param_0, exp);
  frexp_result_vec2 result = {sig, int2(exp)};
  return result;
}

void frexp_db0637() {
  frexp_result_vec2 res = tint_frexp(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  frexp_db0637();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_db0637();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_db0637();
  return;
}
