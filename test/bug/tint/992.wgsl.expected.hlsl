struct tint_symbol {
  float4 value : SV_Target0;
};

tint_symbol frag_main() {
  float b = 0.0f;
  float3 v = float3((b).xxx);
  const tint_symbol tint_symbol_1 = {float4(v, 1.0f)};
  return tint_symbol_1;
}
