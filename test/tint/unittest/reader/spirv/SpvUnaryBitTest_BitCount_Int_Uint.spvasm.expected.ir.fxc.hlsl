SKIP: FAILED

void main_1() {
  int i1 = 30;
  uint2 v2u1 = uint2(10u, 20u);
  int2 v2i1 = int2(30, 40);
  int x_1 = 2;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

