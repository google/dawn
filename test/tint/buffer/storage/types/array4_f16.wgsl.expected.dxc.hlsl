ByteAddressBuffer tint_symbol : register(t0, space0);
RWByteAddressBuffer tint_symbol_1 : register(u1, space0);

void tint_symbol_2(RWByteAddressBuffer buffer, uint offset, float16_t value[4]) {
  float16_t array[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      buffer.Store<float16_t>((offset + (i * 2u)), array[i]);
    }
  }
}

typedef float16_t tint_symbol_4_ret[4];
tint_symbol_4_ret tint_symbol_4(ByteAddressBuffer buffer, uint offset) {
  float16_t arr[4] = (float16_t[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = buffer.Load<float16_t>((offset + (i_1 * 2u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_2(tint_symbol_1, 0u, tint_symbol_4(tint_symbol, 0u));
  return;
}
