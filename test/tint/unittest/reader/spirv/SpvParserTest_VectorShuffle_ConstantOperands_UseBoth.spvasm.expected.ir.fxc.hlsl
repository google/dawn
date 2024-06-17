SKIP: FAILED

void main_1() {
  uint4 x_10 = uint4(3u, 4u, 4u, 3u);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

