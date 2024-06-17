SKIP: FAILED

void main_1() {
  uint u1 = 10u;
  int i1 = 30;
  int2 v2i1 = int2(30, 40);
  uint2 x_1 = uint2(1342177280u, 671088640u);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

