SKIP: FAILED

void main_1() {
  uint2 x_1 = uint2(10u, 20u);
  int2 x_2 = int2(30, 40);
  float2 x_3 = float2(50.0f, 60.0f);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

