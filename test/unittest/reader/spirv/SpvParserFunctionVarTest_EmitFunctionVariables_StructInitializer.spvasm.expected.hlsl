struct S {
  uint field0;
  float field1;
  uint field2[2];
};

void main_1() {
  const uint tint_symbol[2] = {1u, 2u};
  S x_200 = {1u, 1.5f, tint_symbol};
  return;
}

void main() {
  main_1();
  return;
}
