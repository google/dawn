void main_1() {
  const uint2 x_1 = uint2(3u, 4u);
  const uint x_10 = x_1[1];
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
