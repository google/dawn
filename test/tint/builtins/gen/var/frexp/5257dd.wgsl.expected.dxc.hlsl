struct frexp_result_f16 {
  float16_t fract;
  int exp;
};
frexp_result_f16 tint_frexp(float16_t param_0) {
  float16_t exp;
  float16_t fract = sign(param_0) * frexp(param_0, exp);
  frexp_result_f16 result = {fract, int(exp)};
  return result;
}

void frexp_5257dd() {
  float16_t arg_0 = float16_t(1.0h);
  frexp_result_f16 res = tint_frexp(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_5257dd();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_5257dd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_5257dd();
  return;
}
