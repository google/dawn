void step_07cb06() {
  vector<float16_t, 2> arg_0 = (float16_t(0.0h)).xx;
  vector<float16_t, 2> arg_1 = (float16_t(0.0h)).xx;
  vector<float16_t, 2> res = step(arg_0, arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  step_07cb06();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  step_07cb06();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  step_07cb06();
  return;
}
