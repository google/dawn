
int2 tint_bitcast_from_f16(vector<float16_t, 4> src) {
  uint4 v = ((uint4(asuint16(src)) & (65535u).xxxx) << uint4(0u, 16u, 0u, 16u));
  return asint(uint2((v.x | v.y), (v.z | v.w)));
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 4> a = vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(-4.0h));
  int2 b = tint_bitcast_from_f16(a);
}

