struct modf_result_f32 {
  float fract;
  float whole;
};


[numthreads(1, 1, 1)]
void main() {
  float tint_symbol = 1.25f;
  float v = 0.0f;
  float v_1 = modf(tint_symbol, v);
  modf_result_f32 res = {v_1, v};
  float fract = res.fract;
  float whole = res.whole;
}

