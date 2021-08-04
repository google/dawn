struct Output {
  float4 Position;
  float4 color;
};
struct tint_symbol_1 {
  uint VertexIndex : SV_VertexID;
  uint InstanceIndex : SV_InstanceID;
};
struct tint_symbol_2 {
  float4 color : TEXCOORD0;
  float4 Position : SV_Position;
};

Output main_inner(uint VertexIndex, uint InstanceIndex) {
  float2 zv[4] = {float2(0.200000003f, 0.200000003f), float2(0.300000012f, 0.300000012f), float2(-0.100000001f, -0.100000001f), float2(1.100000024f, 1.100000024f)};
  const float z = zv[InstanceIndex].x;
  Output output = (Output)0;
  output.Position = float4(0.5f, 0.5f, z, 1.0f);
  float4 colors[4] = {float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
  output.color = colors[InstanceIndex];
  return output;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const Output inner_result = main_inner(tint_symbol.VertexIndex, tint_symbol.InstanceIndex);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.Position = inner_result.Position;
  wrapper_result.color = inner_result.color;
  return wrapper_result;
}
