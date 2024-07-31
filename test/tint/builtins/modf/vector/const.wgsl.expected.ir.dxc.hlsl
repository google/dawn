struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};


[numthreads(1, 1, 1)]
void main() {
  modf_result_vec2_f32 v = {float2(0.25f, 0.75f), float2(1.0f, 3.0f)};
  modf_result_vec2_f32 res = v;
  float2 fract = res.fract;
  modf_result_vec2_f32 v_1 = v;
  float2 whole = v_1.whole;
}

