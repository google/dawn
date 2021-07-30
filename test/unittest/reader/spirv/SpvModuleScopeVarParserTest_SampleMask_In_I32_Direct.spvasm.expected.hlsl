static int x_1[1] = (int[1])0;

void main_1() {
  const int x_3 = x_1[0];
  return;
}

struct tint_symbol_1 {
  uint x_1_param : SV_Coverage;
};

void main(tint_symbol_1 tint_symbol) {
  const uint x_1_param = tint_symbol.x_1_param;
  x_1[0] = asint(x_1_param);
  main_1();
  return;
}
