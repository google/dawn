void round_e1bba2() {
  vector<float16_t, 3> res = (float16_t(3.0h)).xxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  round_e1bba2();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  round_e1bba2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  round_e1bba2();
  return;
}
