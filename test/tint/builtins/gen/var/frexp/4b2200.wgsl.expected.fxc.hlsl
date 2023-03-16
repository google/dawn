struct frexp_result_f32 {
  float fract;
  int exp;
};
frexp_result_f32 tint_frexp(float param_0) {
  float exp;
  float fract = sign(param_0) * frexp(param_0, exp);
  frexp_result_f32 result = {fract, int(exp)};
  return result;
}

void frexp_4b2200() {
  float arg_0 = 1.0f;
  frexp_result_f32 res = tint_frexp(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_4b2200();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_4b2200();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_4b2200();
  return;
}
