ByteAddressBuffer G : register(t0);

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_1 = 0u;
  G.GetDimensions(tint_symbol_1);
  const uint tint_symbol_2 = ((tint_symbol_1 - 0u) / 4u);
  const uint l1 = tint_symbol_2;
  const uint l2 = tint_symbol_2;
  return;
}
