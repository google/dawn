ByteAddressBuffer sb_ro : register(t1, space0);

void arrayLength_1588cd() {
  uint tint_symbol_2 = 0u;
  sb_ro.GetDimensions(tint_symbol_2);
  const uint tint_symbol_3 = ((tint_symbol_2 - 0u) / 4u);
  uint res = tint_symbol_3;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  arrayLength_1588cd();
  const tint_symbol tint_symbol_4 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_4;
}

void fragment_main() {
  arrayLength_1588cd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  arrayLength_1588cd();
  return;
}
