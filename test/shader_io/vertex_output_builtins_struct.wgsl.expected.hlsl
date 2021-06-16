struct VertexOutputs {
  float4 position;
};
struct tint_symbol {
  float4 position : SV_Position;
};

tint_symbol main() {
  const VertexOutputs tint_symbol_1 = {float4(1.0f, 2.0f, 3.0f, 4.0f)};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.position};
  return tint_symbol_2;
}
