struct VertexInputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
};
struct tint_symbol_1 {
  int loc0 : TEXCOORD0;
  uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const VertexInputs inputs = {tint_symbol.loc0, tint_symbol.loc1, tint_symbol.loc2, tint_symbol.loc3};
  const int i = inputs.loc0;
  const uint u = inputs.loc1;
  const float f = inputs.loc2;
  const float4 v = inputs.loc3;
  const tint_symbol_2 tint_symbol_3 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_3;
}
