
int tint_bitcast_from_f16(vector<float16_t, 2> src) {
  uint2 v = ((uint2(asuint16(src)) & (65535u).xx) << uint2(0u, 16u));
  return asint((v.x | v.y));
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  int b = tint_bitcast_from_f16(a);
}

