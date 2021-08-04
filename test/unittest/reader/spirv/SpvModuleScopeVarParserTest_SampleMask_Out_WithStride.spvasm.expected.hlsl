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

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_1[0]};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_1_1 = inner_result.x_1_1;
  return wrapper_result;
}
