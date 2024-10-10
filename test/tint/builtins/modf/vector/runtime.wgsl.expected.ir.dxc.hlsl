struct modf_result_vec2_f32 {
  float2 fract;
  float2 whole;
};


[numthreads(1, 1, 1)]
void main() {
  float2 tint_symbol = float2(1.25f, 3.75f);
  float2 v = (0.0f).xx;
  float2 v_1 = modf(tint_symbol, v);
  modf_result_vec2_f32 v_2 = {v_1, v};
  modf_result_vec2_f32 res = v_2;
  float2 fract = res.fract;
  modf_result_vec2_f32 v_3 = v_2;
  float2 whole = v_3.whole;
}

