SKIP: FAILED

struct frexp_result_vec4_f16 {
  vector<float16_t, 4> fract;
  int4 exp;
};
void frexp_3dd21e() {
  frexp_result_vec4_f16 res = {(float16_t(0.5h)).xxxx, (1).xxxx};
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_3dd21e();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_3dd21e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_3dd21e();
  return;
}
