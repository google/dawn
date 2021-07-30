void main_1() {
  uint x_35[5] = (uint[5])0;
  uint x_2_1[5] = x_35;
  x_2_1[3u] = 20u;
  const uint x_2[5] = x_2_1;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
