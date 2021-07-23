struct frexp_result_vec4 {
  float4 sig;
  int4 exp;
};
frexp_result_vec4 tint_frexp(float4 param_0) {
  float4 exp;
  float4 sig = frexp(param_0, exp);
  frexp_result_vec4 result = {sig, int4(exp)};
  return result;
}

void frexp_d80367() {
  frexp_result_vec4 res = tint_frexp(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  frexp_d80367();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_d80367();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_d80367();
  return;
}
