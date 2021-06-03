
ByteAddressBuffer sb : register(t0, space0);

void arrayLength_647a40() {
  uint tint_symbol_1 = 0u;
  sb.GetDimensions(tint_symbol_1);
  const uint tint_symbol_2 = ((tint_symbol_1 - 0u) / 4u);
  uint res = tint_symbol_2;
}

void vertex_main() {
  arrayLength_647a40();
  return;
}

void fragment_main() {
  arrayLength_647a40();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  arrayLength_647a40();
  return;
}

