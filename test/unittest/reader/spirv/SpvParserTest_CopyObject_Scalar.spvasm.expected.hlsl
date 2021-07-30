void main_1() {
  const uint x_2 = 3u;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
