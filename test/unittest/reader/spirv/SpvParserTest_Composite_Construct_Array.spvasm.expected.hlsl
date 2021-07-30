void main_1() {
  const uint x_1[5] = {10u, 20u, 3u, 4u, 5u};
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
