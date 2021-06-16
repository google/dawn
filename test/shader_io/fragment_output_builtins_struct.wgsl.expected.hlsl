struct FragmentOutputs {
  float frag_depth;
  uint sample_mask;
};
struct tint_symbol {
  float frag_depth : SV_Depth;
  uint sample_mask : SV_Coverage;
};

tint_symbol main() {
  const FragmentOutputs tint_symbol_1 = {1.0f, 1u};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.frag_depth, tint_symbol_1.sample_mask};
  return tint_symbol_2;
}
