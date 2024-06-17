SKIP: FAILED

void main_1() {
  uint2 x_10 = uint2(4u, 3u);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

