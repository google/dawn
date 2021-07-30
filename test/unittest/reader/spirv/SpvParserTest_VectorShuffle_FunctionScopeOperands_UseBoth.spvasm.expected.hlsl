void main_1() {
  const uint2 x_1 = uint2(3u, 4u);
  const uint2 x_2 = (uint2(4u, 3u) + uint2(3u, 4u));
  const uint4 x_10 = uint4(x_2.y, x_2.x, x_1.y, x_1.x);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
