struct frexp_result {
  float fract;
  int exp;
};
[numthreads(1, 1, 1)]
void main() {
  const frexp_result res = {0.61500001f, 1};
  const int exp = res.exp;
  const float fract = res.fract;
  return;
}
