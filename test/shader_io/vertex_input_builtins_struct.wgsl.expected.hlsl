struct VertexInputs {
  uint vertex_index;
  uint instance_index;
};
struct tint_symbol_1 {
  uint vertex_index : SV_VertexID;
  uint instance_index : SV_InstanceID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const VertexInputs inputs = {tint_symbol.vertex_index, tint_symbol.instance_index};
  const uint foo = (inputs.vertex_index + inputs.instance_index);
  const tint_symbol_2 tint_symbol_3 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_3;
}
