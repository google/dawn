static uint x_1 = 0u;

void main_1() {
  const uint x_2 = x_1;
  return;
}

struct tint_symbol_1 {
  uint x_1_param : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  const uint x_1_param = tint_symbol.x_1_param;
  x_1 = x_1_param;
  main_1();
  return;
}
