struct VertexInputs0 {
  uint vertex_index;
  int loc0;
};
struct VertexInputs1 {
  float loc2;
  float4 loc3;
};
struct tint_symbol_1 {
  int loc0 : TEXCOORD0;
  uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
  uint vertex_index : SV_VertexID;
  uint instance_index : SV_InstanceID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const VertexInputs0 inputs0 = {tint_symbol.vertex_index, tint_symbol.loc0};
  const uint loc1 = tint_symbol.loc1;
  const uint instance_index = tint_symbol.instance_index;
  const VertexInputs1 inputs1 = {tint_symbol.loc2, tint_symbol.loc3};
  const uint foo = (inputs0.vertex_index + instance_index);
  const int i = inputs0.loc0;
  const uint u = loc1;
  const float f = inputs1.loc2;
  const float4 v = inputs1.loc3;
  const tint_symbol_2 tint_symbol_3 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_3;
}
