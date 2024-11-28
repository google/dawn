
ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
[numthreads(1, 1, 1)]
void main() {
  uint v = 0u;
  tint_symbol_1.GetDimensions(v);
  uint v_1 = ((v / 2u) - 1u);
  uint v_2 = (min(uint(int(0)), v_1) * 2u);
  uint v_3 = 0u;
  tint_symbol.GetDimensions(v_3);
  uint v_4 = ((v_3 / 2u) - 1u);
  tint_symbol_1.Store<float16_t>((0u + v_2), tint_symbol.Load<float16_t>((0u + (min(uint(int(0)), v_4) * 2u))));
}

