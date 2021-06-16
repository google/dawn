struct tint_symbol_1 {
  float4 position : SV_Position;
  bool front_facing : SV_IsFrontFace;
  uint sample_index : SV_SampleIndex;
  uint sample_mask : SV_Coverage;
};

void main(tint_symbol_1 tint_symbol) {
  const float4 position = tint_symbol.position;
  const bool front_facing = tint_symbol.front_facing;
  const uint sample_index = tint_symbol.sample_index;
  const uint sample_mask = tint_symbol.sample_mask;
  if (front_facing) {
    const float4 foo = position;
    const uint bar = (sample_index + sample_mask);
  }
  return;
}
