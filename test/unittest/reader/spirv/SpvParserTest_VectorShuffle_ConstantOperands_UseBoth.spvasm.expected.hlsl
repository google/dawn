void main_1() {
  const uint4 x_10 = uint4(uint2(4u, 3u).y, uint2(4u, 3u).x, uint2(3u, 4u).y, uint2(3u, 4u).x);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
