struct FragmentInputs0 {
  float4 position;
  int loc0;
};
struct FragmentInputs1 {
  float4 loc3;
  uint sample_mask;
};
struct tint_symbol_1 {
  int loc0 : TEXCOORD0;
  uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
  float4 position : SV_Position;
  bool front_facing : SV_IsFrontFace;
  uint sample_index : SV_SampleIndex;
  uint sample_mask : SV_Coverage;
};

void main(tint_symbol_1 tint_symbol) {
  const FragmentInputs0 inputs0 = {tint_symbol.position, tint_symbol.loc0};
  const bool front_facing = tint_symbol.front_facing;
  const uint loc1 = tint_symbol.loc1;
  const uint sample_index = tint_symbol.sample_index;
  const FragmentInputs1 inputs1 = {tint_symbol.loc3, tint_symbol.sample_mask};
  const float loc2 = tint_symbol.loc2;
  if (front_facing) {
    const float4 foo = inputs0.position;
    const uint bar = (sample_index + inputs1.sample_mask);
    const int i = inputs0.loc0;
    const uint u = loc1;
    const float f = loc2;
    const float4 v = inputs1.loc3;
  }
  return;
}
