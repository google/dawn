struct VSInput {
  float4 val;
};
struct VSOutputs {
  int result;
  float4 position;
};
struct tint_symbol_1 {
  float4 val : TEXCOORD0;
};
struct tint_symbol_2 {
  nointerpolation int result : TEXCOORD0;
  float4 position : SV_Position;
};

VSOutputs vsMain_inner(VSInput vtxIn) {
  VSOutputs tint_symbol_4 = {1, vtxIn.val};
  return tint_symbol_4;
}

tint_symbol_2 vsMain(tint_symbol_1 tint_symbol) {
  VSInput tint_symbol_3 = {tint_symbol.val};
  VSOutputs inner_result = vsMain_inner(tint_symbol_3);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.result = inner_result.result;
  wrapper_result.position = inner_result.position;
  return wrapper_result;
}
