struct FragmentOutputs {
  int loc0;
  float frag_depth;
  uint loc1;
  float loc2;
  uint sample_mask;
  float4 loc3;
};
struct tint_symbol {
  int loc0 : SV_Target0;
  uint loc1 : SV_Target1;
  float loc2 : SV_Target2;
  float4 loc3 : SV_Target3;
  float frag_depth : SV_Depth;
  uint sample_mask : SV_Coverage;
};

tint_symbol main() {
  const FragmentOutputs tint_symbol_1 = {1, 2.0f, 1u, 1.0f, 2u, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.loc0, tint_symbol_1.loc1, tint_symbol_1.loc2, tint_symbol_1.loc3, tint_symbol_1.frag_depth, tint_symbol_1.sample_mask};
  return tint_symbol_2;
}
