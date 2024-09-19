struct frexp_result_f32 {
  float fract;
  int exp;
};


[numthreads(1, 1, 1)]
void main() {
  frexp_result_f32 v = {0.61500000953674316406f, int(1)};
  frexp_result_f32 res = v;
  int exp = res.exp;
  frexp_result_f32 v_1 = v;
  float fract = v_1.fract;
}

