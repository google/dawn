vector<float16_t, 4> tint_degrees(vector<float16_t, 4> param_0) {
  return param_0 * 57.295779513082322865;
}

void degrees_3055d3() {
  vector<float16_t, 4> arg_0 = (float16_t(0.0h)).xxxx;
  vector<float16_t, 4> res = tint_degrees(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  degrees_3055d3();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  degrees_3055d3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  degrees_3055d3();
  return;
}
