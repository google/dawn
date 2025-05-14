RWByteAddressBuffer s : register(u0);

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_1 = 0u;
  s.GetDimensions(tint_symbol_1);
  uint tint_symbol_2 = (tint_symbol_1 / 4u);
  uint q = 0u;
  uint2 tint_symbol_3[2] = {uint2(1u, 2u), uint2(3u, 4u)};
  s.Store((4u * min(0u, (tint_symbol_2 - 1u))), asuint(tint_symbol_3[min(q, 1u)].x));
  return;
}
