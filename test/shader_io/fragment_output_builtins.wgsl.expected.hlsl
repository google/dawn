struct tint_symbol {
  float value : SV_Depth;
};

tint_symbol main1() {
  const tint_symbol tint_symbol_2 = {1.0f};
  return tint_symbol_2;
}

struct tint_symbol_1 {
  uint value : SV_Coverage;
};

tint_symbol_1 main2() {
  const tint_symbol_1 tint_symbol_3 = {1u};
  return tint_symbol_3;
}
