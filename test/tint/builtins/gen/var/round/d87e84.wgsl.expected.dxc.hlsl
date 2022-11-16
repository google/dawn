void round_d87e84() {
  vector<float16_t, 2> arg_0 = (float16_t(3.3984375h)).xx;
  vector<float16_t, 2> res = round(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  round_d87e84();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  round_d87e84();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  round_d87e84();
  return;
}
