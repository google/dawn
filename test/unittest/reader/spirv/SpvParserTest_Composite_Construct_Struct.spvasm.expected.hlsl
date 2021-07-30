struct S {
  float2 field0;
  uint field1;
  int field2;
};

void main_1() {
  const S x_1 = {float2(50.0f, 60.0f), 5u, 30};
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
