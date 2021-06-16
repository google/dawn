struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol main() {
  float3 light = float3(1.200000048f, 1.0f, 2.0f);
  float3 negative_light = -(light);
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}
