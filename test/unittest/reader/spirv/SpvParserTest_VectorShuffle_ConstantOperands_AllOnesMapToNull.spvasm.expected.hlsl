void main_1() {
  const uint2 x_1 = uint2(4u, 3u);
  const uint2 x_10 = uint2(0u, x_1.y);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
