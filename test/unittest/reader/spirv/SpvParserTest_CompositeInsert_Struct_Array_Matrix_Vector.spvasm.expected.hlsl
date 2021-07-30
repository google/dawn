struct S_1 {
  uint field0;
  float3x2 field1[3];
};

void main_1() {
  S_1 x_37 = (S_1)0;
  S_1 x_2_1 = x_37;
  x_2_1.field1[2u][0u].y = 70.0f;
  const S_1 x_2 = x_2_1;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
