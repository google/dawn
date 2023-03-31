ByteAddressBuffer arr : register(t0);

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_1 = 0u;
  arr.GetDimensions(tint_symbol_1);
  const uint tint_symbol_2 = (tint_symbol_1 / 4u);
  const uint len = tint_symbol_2;
  return;
}
