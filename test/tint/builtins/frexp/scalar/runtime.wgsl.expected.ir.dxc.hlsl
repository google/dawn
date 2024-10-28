struct frexp_result_f32 {
  float fract;
  int exp;
};


[numthreads(1, 1, 1)]
void main() {
  float tint_symbol = 1.25f;
  float v = 0.0f;
  float v_1 = frexp(tint_symbol, v);
  float v_2 = (float(sign(tint_symbol)) * v_1);
  frexp_result_f32 res = {v_2, int(v)};
  float fract = res.fract;
  int exp = res.exp;
}

