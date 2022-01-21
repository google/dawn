struct Input {
  float4 color;
};
struct Output {
  float4 color;
};
struct tint_symbol_2 {
  float4 color : TEXCOORD0;
};
struct tint_symbol_3 {
  float4 color : SV_Target0;
};

Output main_inner(Input tint_symbol) {
  const Output tint_symbol_4 = {tint_symbol.color};
  return tint_symbol_4;
}

tint_symbol_3 main(tint_symbol_2 tint_symbol_1) {
  const Input tint_symbol_5 = {tint_symbol_1.color};
  const Output inner_result = main_inner(tint_symbol_5);
  tint_symbol_3 wrapper_result = (tint_symbol_3)0;
  wrapper_result.color = inner_result.color;
  return wrapper_result;
}
