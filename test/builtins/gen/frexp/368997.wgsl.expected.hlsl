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

void frexp_368997() {
  frexp_result_vec3 res = tint_frexp(float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_368997();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_368997();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_368997();
  return;
}
