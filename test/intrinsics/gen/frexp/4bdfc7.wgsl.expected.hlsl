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

void frexp_4bdfc7() {
  frexp_result_vec2 res = tint_frexp(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_4bdfc7();
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_4bdfc7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_4bdfc7();
  return;
}
