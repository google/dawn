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

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const uint VertexIndex = tint_symbol.VertexIndex;
  const uint InstanceIndex = tint_symbol.InstanceIndex;
  const float2 zv[4] = {float2(0.200000003f, 0.200000003f), float2(0.300000012f, 0.300000012f), float2(-0.100000001f, -0.100000001f), float2(1.100000024f, 1.100000024f)};
  const float z = zv[InstanceIndex].x;
  Output output = {float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)};
  output.Position = float4(0.5f, 0.5f, z, 1.0f);
  const float4 colors[4] = {float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f), float4(1.0f, 1.0f, 1.0f, 1.0f)};
  output.color = colors[InstanceIndex];
  const tint_symbol_2 tint_symbol_3 = {output.color, output.Position};
  return tint_symbol_3;
}

