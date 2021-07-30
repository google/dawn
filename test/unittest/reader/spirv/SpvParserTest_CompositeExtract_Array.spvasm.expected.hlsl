void main_1() {
  uint x_35[5] = (uint[5])0;
  const uint x_2 = x_35[3u];
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
