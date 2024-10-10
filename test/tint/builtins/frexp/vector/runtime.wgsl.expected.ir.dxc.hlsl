struct frexp_result_vec2_f32 {
  float2 fract;
  int2 exp;
};


[numthreads(1, 1, 1)]
void main() {
  float2 tint_symbol = float2(1.25f, 3.75f);
  float2 v = (0.0f).xx;
  float2 v_1 = frexp(tint_symbol, v);
  float2 v_2 = float2(sign(tint_symbol));
  v = (v_2 * v);
  frexp_result_vec2_f32 v_3 = {v_1, int2(v)};
  frexp_result_vec2_f32 res = v_3;
  float2 fract = res.fract;
  frexp_result_vec2_f32 v_4 = v_3;
  int2 exp = v_4.exp;
}

