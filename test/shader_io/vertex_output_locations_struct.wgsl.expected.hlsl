struct VertexOutputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
  float4 position;
};
struct tint_symbol {
  int loc0 : TEXCOORD0;
  uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
  float4 position : SV_Position;
};

tint_symbol main() {
  const VertexOutputs tint_symbol_1 = {1, 1u, 1.0f, float4(1.0f, 2.0f, 3.0f, 4.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.loc0, tint_symbol_1.loc1, tint_symbol_1.loc2, tint_symbol_1.loc3, tint_symbol_1.position};
  return tint_symbol_2;
}
