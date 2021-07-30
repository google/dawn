struct S {
  uint field0;
  float field1;
  uint field2[2];
};

static S x_200 = {1u, 1.5f, {1u, 2u}};

void main_1() {
  return;
}

void main() {
  main_1();
  return;
}
