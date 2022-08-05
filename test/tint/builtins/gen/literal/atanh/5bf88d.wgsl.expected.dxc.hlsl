vector<float16_t, 2> tint_atanh(vector<float16_t, 2> x) {
  return (log(((float16_t(1.0h) + x) / (float16_t(1.0h) - x))) * float16_t(0.5h));
}

void atanh_5bf88d() {
  vector<float16_t, 2> res = tint_atanh((float16_t(0.0h)).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atanh_5bf88d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atanh_5bf88d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atanh_5bf88d();
  return;
}
