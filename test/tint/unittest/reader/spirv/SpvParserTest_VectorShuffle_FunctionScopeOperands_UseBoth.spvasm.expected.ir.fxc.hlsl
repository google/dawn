SKIP: FAILED

void main_1() {
  uint4 x_10 = uint4(7u, 7u, 4u, 3u);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

