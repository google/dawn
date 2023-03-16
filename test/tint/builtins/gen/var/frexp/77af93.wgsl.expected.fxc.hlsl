struct frexp_result_vec4_f32 {
  float4 fract;
  int4 exp;
};
frexp_result_vec4_f32 tint_frexp(float4 param_0) {
  float4 exp;
  float4 fract = sign(param_0) * frexp(param_0, exp);
  frexp_result_vec4_f32 result = {fract, int4(exp)};
  return result;
}

void frexp_77af93() {
  float4 arg_0 = (1.0f).xxxx;
  frexp_result_vec4_f32 res = tint_frexp(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_77af93();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_77af93();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_77af93();
  return;
}
