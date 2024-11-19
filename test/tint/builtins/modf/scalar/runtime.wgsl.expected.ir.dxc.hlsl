struct modf_result_f32 {
  float fract;
  float whole;
};


[numthreads(1, 1, 1)]
void main() {
  float tint_symbol = 1.25f;
  float v = 0.0f;
  modf_result_f32 res = {modf(tint_symbol, v), v};
  float fract = res.fract;
  float whole = res.whole;
}

