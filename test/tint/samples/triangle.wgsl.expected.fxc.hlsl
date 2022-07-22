struct tint_symbol_1 {
  uint VertexIndex : SV_VertexID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

float4 vtx_main_inner(uint VertexIndex) {
  const float2 tint_symbol_4[3] = {float2(0.0f, 0.5f), (-0.5f).xx, float2(0.5f, -0.5f)};
  return float4(tint_symbol_4[VertexIndex], 0.0f, 1.0f);
}

tint_symbol_2 vtx_main(tint_symbol_1 tint_symbol) {
  const float4 inner_result = vtx_main_inner(tint_symbol.VertexIndex);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

struct tint_symbol_3 {
  float4 value : SV_Target0;
};

float4 frag_main_inner() {
  return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

tint_symbol_3 frag_main() {
  const float4 inner_result_1 = frag_main_inner();
  tint_symbol_3 wrapper_result_1 = (tint_symbol_3)0;
  wrapper_result_1.value = inner_result_1;
  return wrapper_result_1;
}
