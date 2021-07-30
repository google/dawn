struct S {
  float2 field0;
  uint field1;
  int field2;
};

void main_1() {
  S x_35 = (S)0;
  const int x_2 = x_35.field2;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
