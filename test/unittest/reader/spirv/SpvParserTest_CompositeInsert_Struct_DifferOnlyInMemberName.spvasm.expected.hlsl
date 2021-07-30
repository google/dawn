struct S {
  uint algo;
};
struct S_1 {
  uint rithm;
};

void main_1() {
  S var0 = (S)0;
  S_1 var1 = (S_1)0;
  S x_2_1 = var0;
  x_2_1.algo = 10u;
  const S x_2 = x_2_1;
  S_1 x_4_1 = var1;
  x_4_1.rithm = 11u;
  const S_1 x_4 = x_4_1;
  return;
}

void main() {
  main_1();
  return;
}
