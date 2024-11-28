
ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
[numthreads(1, 1, 1)]
void main() {
  uint v = 0u;
  tint_symbol_1.GetDimensions(v);
  uint v_1 = ((v / 4u) - 1u);
  uint v_2 = (min(uint(int(0)), v_1) * 4u);
  uint v_3 = 0u;
  tint_symbol.GetDimensions(v_3);
  uint v_4 = ((v_3 / 4u) - 1u);
  tint_symbol_1.Store((0u + v_2), asuint(asfloat(tint_symbol.Load((0u + (min(uint(int(0)), v_4) * 4u))))));
}

