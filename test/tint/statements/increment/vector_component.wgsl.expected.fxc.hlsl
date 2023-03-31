[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

RWByteAddressBuffer a : register(u0);

void main() {
  const int tint_symbol_1 = 1;
  a.Store((4u * uint(tint_symbol_1)), asuint((a.Load((4u * uint(tint_symbol_1))) + 1u)));
  a.Store(8u, asuint((a.Load(8u) + 1u)));
}
