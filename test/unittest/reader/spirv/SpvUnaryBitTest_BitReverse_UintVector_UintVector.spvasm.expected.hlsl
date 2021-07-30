void main_1() {
  const uint u1 = 10u;
  const int i1 = 30;
  const uint2 v2u1 = uint2(10u, 20u);
  const int2 v2i1 = int2(30, 40);
  const uint2 x_1 = reversebits(v2u1);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
