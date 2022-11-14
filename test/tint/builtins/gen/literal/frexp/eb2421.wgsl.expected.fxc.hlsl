struct frexp_result_vec2 {
  float2 fract;
  int2 exp;
};
frexp_result_vec2 tint_frexp(float2 param_0) {
  float2 exp;
  float2 fract = frexp(param_0, exp);
  frexp_result_vec2 result = {fract, int2(exp)};
  return result;
}

void frexp_eb2421() {
  frexp_result_vec2 res = tint_frexp((1.0f).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_eb2421();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_eb2421();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_eb2421();
  return;
}
