SKIP: FAILED

void main_1() {
  uint[5] x_1 = {10u, 20u, 3u, 4u, 5u};
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

