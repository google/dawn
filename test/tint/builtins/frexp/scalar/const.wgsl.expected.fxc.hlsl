struct frexp_result_f32 {
  float fract;
  int exp;
};
[numthreads(1, 1, 1)]
void main() {
  const frexp_result_f32 res = {0.625f, 1};
  const float fract = res.fract;
  const int exp = res.exp;
  return;
}
