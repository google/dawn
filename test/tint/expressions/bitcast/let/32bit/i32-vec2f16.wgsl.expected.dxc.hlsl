
vector<float16_t, 2> tint_bitcast_to_f16(int src) {
  uint v = asuint(src);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

[numthreads(1, 1, 1)]
void f() {
  int a = int(1073757184);
  vector<float16_t, 2> b = tint_bitcast_to_f16(a);
}

