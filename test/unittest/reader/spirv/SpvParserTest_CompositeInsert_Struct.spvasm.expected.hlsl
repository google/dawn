struct S {
  float2 field0;
  uint field1;
  int field2;
};

void main_1() {
  S x_35 = (S)0;
  S x_2_1 = x_35;
  x_2_1.field2 = 30;
  const S x_2 = x_2_1;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
