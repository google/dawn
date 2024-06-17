SKIP: FAILED

void main_1() {
  uint x_1 = 3u;
  uint x_2 = x_1;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

