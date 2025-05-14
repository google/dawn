RWByteAddressBuffer s : register(u0);

struct A {
  uint2 b;
  uint c;
};

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_1 = 0u;
  s.GetDimensions(tint_symbol_1);
  uint tint_symbol_2 = (tint_symbol_1 / 4u);
  uint q = 0u;
  A tint_symbol_3[2] = {{uint2(1u, 2u), 3u}, {uint2(4u, 5u), 6u}};
  s.Store((4u * min(0u, (tint_symbol_2 - 1u))), asuint(tint_symbol_3[min(q, 1u)].b.x));
  return;
}
