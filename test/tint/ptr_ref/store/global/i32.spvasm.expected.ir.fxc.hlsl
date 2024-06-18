SKIP: FAILED

void main_1() {
  I = 123;
  I = 123;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

