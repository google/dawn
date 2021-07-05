typedef float f1_ret[4];
f1_ret f1() {
  const float tint_symbol[4] = (float[4])0;
  return tint_symbol;
}

typedef float f2_ret[3][4];
f2_ret f2() {
  const float tint_symbol_1[3][4] = {f1(), f1(), f1()};
  return tint_symbol_1;
}

typedef float f3_ret[2][3][4];
f3_ret f3() {
  const float tint_symbol_2[2][3][4] = {f2(), f2()};
  return tint_symbol_2;
}

[numthreads(1, 1, 1)]
void main() {
  const float a1[4] = f1();
  const float a2[3][4] = f2();
  const float a3[2][3][4] = f3();
  return;
}
