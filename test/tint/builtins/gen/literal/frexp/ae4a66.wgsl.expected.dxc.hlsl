struct frexp_result_vec3_f16 {
  vector<float16_t, 3> fract;
  int3 exp;
};
void frexp_ae4a66() {
  frexp_result_vec3_f16 res = {(float16_t(0.5h)).xxx, (1).xxx};
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  frexp_ae4a66();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  frexp_ae4a66();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  frexp_ae4a66();
  return;
}
