struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};
[numthreads(1, 1, 1)]
void main() {
  const modf_result_vec2_f32 res = {float2(0.25f, 0.75f), float2(1.0f, 3.0f)};
  const float2 fract = res.fract;
  const float2 whole = res.whole;
  return;
}
