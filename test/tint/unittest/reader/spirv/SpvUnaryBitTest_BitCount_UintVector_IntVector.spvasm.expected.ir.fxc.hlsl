SKIP: FAILED

void main_1() {
  uint u1 = 10u;
  int i1 = 30;
  uint2 v2u1 = uint2(10u, 20u);
  uint2 x_1 = uint2(4u, 2u);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

