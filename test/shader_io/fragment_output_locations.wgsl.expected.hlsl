struct tint_symbol {
  int value : SV_Target0;
};

tint_symbol main0() {
  const tint_symbol tint_symbol_4 = {1};
  return tint_symbol_4;
}

struct tint_symbol_1 {
  uint value : SV_Target1;
};

tint_symbol_1 main1() {
  const tint_symbol_1 tint_symbol_5 = {1u};
  return tint_symbol_5;
}

struct tint_symbol_2 {
  float value : SV_Target2;
};

tint_symbol_2 main2() {
  const tint_symbol_2 tint_symbol_6 = {1.0f};
  return tint_symbol_6;
}

struct tint_symbol_3 {
  float4 value : SV_Target3;
};

tint_symbol_3 main3() {
  const tint_symbol_3 tint_symbol_7 = {float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return tint_symbol_7;
}
