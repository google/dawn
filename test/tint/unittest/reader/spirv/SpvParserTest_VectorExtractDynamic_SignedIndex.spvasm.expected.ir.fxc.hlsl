SKIP: FAILED

void main_1() {
  uint x_10 = 4u;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

