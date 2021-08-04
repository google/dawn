static uint x_1 = 0u;

void main_1() {
  const uint x_2 = x_1;
  return;
}

struct tint_symbol_1 {
  uint x_1_param : SV_SampleIndex;
};

void main_inner(uint x_1_param) {
  x_1 = x_1_param;
  main_1();
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x_1_param);
  return;
}
