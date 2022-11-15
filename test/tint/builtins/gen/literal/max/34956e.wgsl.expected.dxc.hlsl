void max_34956e() {
  vector<float16_t, 2> res = (float16_t(1.0h)).xx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  max_34956e();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  max_34956e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_34956e();
  return;
}
