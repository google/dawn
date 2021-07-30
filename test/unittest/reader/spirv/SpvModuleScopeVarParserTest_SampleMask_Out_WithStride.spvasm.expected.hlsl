static uint x_1[1] = (uint[1])0;

void main_1() {
  x_1[0] = 0u;
  return;
}

struct main_out {
  uint x_1_1;
};
struct tint_symbol {
  uint x_1_1 : SV_Coverage;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_1[0]};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.x_1_1};
  return tint_symbol_2;
}
