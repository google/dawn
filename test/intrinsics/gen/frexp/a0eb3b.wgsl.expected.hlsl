struct frexp_result_vec3 {
  float3 sig;
  int3 exp;
};
frexp_result_vec3 tint_frexp(float3 param_0) {
  float3 exp;
  float3 sig = frexp(param_0, exp);
  frexp_result_vec3 result = {sig, int3(exp)};
  return result;
}

void frexp_a0eb3b() {
  frexp_result_vec3 res = tint_frexp(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  frexp_a0eb3b();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  frexp_a0eb3b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_a0eb3b();
  return;
}
