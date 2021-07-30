void set_uint2(inout uint2 vec, int idx, uint val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void main_1() {
  const uint2 x_1 = uint2(3u, 4u);
  const uint x_2 = 3u;
  const int x_3 = 1;
  uint2 x_10_1 = x_1;
  set_uint2(x_10_1, x_3, x_2);
  const uint2 x_10 = x_10_1;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
