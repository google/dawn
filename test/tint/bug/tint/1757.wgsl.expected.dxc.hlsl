bug/tint/1757.wgsl:6:25 warning: use of deprecated language feature: 'sig' has been renamed to 'fract'
    let sig : f32 = res.sig;
                        ^^^

struct frexp_result {
  float fract;
  int exp;
};
[numthreads(1, 1, 1)]
void main() {
  const frexp_result res = {0.61500001f, 1};
  const int exp = res.exp;
  const float sig = res.fract;
  return;
}
