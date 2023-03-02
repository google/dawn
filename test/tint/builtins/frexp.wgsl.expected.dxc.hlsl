struct frexp_result_f32 {
  float fract;
  int exp;
};
[numthreads(1, 1, 1)]
void main() {
  const frexp_result_f32 res = {0.61500000953674316406f, 1};
  const int exp = res.exp;
  const float fract = res.fract;
  return;
}
