static uint x_1[1] = (uint[1])0;

void main_1() {
  const uint x_3 = x_1[0];
  return;
}

struct tint_symbol_1 {
  uint x_1_param : SV_Coverage;
};

void main_inner(uint x_1_param) {
  x_1[0] = x_1_param;
  main_1();
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x_1_param);
  return;
}
