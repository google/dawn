SKIP: FAILED

void main_1() {
  uint u1 = 10u;
  int i1 = 30;
  int2 v2i1 = int2(30, 40);
  int2 x_1 = (2).xx;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

