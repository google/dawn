struct tint_symbol_1 {
  int VertexIndex : SV_VertexID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};
struct tint_symbol_3 {
  float4 value : SV_Target0;
};

static const float2 pos[3] = {float2(0.0f, 0.5f), float2(-0.5f, -0.5f), float2(0.5f, -0.5f)};
tint_symbol_2 vtx_main(tint_symbol_1 tint_symbol) {
  const int VertexIndex = tint_symbol.VertexIndex;
  const tint_symbol_2 tint_symbol_4 = {float4(pos[VertexIndex], 0.0f, 1.0f)};
  return tint_symbol_4;
}

tint_symbol_3 frag_main() {
  const tint_symbol_3 tint_symbol_5 = {float4(1.0f, 0.0f, 0.0f, 1.0f)};
  return tint_symbol_5;
}

