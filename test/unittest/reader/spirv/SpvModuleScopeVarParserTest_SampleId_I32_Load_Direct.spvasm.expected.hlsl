static int x_1 = 0;

void main_1() {
  const int x_2 = x_1;
  return;
}

struct tint_symbol_1 {
  uint x_1_param : SV_SampleIndex;
};

void main_inner(uint x_1_param) {
  x_1 = asint(x_1_param);
  main_1();
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x_1_param);
  return;
}
