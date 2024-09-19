struct frexp_result_f32 {
  float fract;
  int exp;
};


[numthreads(1, 1, 1)]
void main() {
  frexp_result_f32 v = {0.625f, int(1)};
  frexp_result_f32 res = v;
  float fract = res.fract;
  frexp_result_f32 v_1 = v;
  int exp = v_1.exp;
}

