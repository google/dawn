struct Input {
  float4 position;
};
struct Output {
  float4 position;
};
struct tint_symbol_2 {
  float4 position : TEXCOORD0;
};
struct tint_symbol_3 {
  float4 position : SV_Position;
};

Output main_inner(Input tint_symbol) {
  const Output tint_symbol_4 = {tint_symbol.position};
  return tint_symbol_4;
}

tint_symbol_3 main(tint_symbol_2 tint_symbol_1) {
  const Input tint_symbol_5 = {tint_symbol_1.position};
  const Output inner_result = main_inner(tint_symbol_5);
  tint_symbol_3 wrapper_result = (tint_symbol_3)0;
  wrapper_result.position = inner_result.position;
  return wrapper_result;
}
