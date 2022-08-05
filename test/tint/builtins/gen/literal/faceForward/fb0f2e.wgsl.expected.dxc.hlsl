void faceForward_fb0f2e() {
  vector<float16_t, 2> res = faceforward((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  faceForward_fb0f2e();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  faceForward_fb0f2e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  faceForward_fb0f2e();
  return;
}
