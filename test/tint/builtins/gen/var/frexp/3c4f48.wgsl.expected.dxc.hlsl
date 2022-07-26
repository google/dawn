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

void frexp_3c4f48() {
  float4 arg_0 = (1.0f).xxxx;
  frexp_result_vec4 res = tint_frexp(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_3c4f48();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_3c4f48();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_3c4f48();
  return;
}
