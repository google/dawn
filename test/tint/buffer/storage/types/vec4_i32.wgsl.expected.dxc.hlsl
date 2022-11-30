ByteAddressBuffer tint_symbol : register(t0, space0);
RWByteAddressBuffer tint_symbol_1 : register(u1, space0);

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1.Store4(0u, asuint(asint(tint_symbol.Load4(0u))));
  return;
}
