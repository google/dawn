void fma_bf21b6() {
  vector<float16_t, 2> arg_0 = (float16_t(0.0h)).xx;
  vector<float16_t, 2> arg_1 = (float16_t(0.0h)).xx;
  vector<float16_t, 2> arg_2 = (float16_t(0.0h)).xx;
  vector<float16_t, 2> res = mad(arg_0, arg_1, arg_2);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  fma_bf21b6();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  fma_bf21b6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fma_bf21b6();
  return;
}
