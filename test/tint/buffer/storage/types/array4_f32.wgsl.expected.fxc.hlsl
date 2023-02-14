ByteAddressBuffer tint_symbol : register(t0, space0);
RWByteAddressBuffer tint_symbol_1 : register(u1, space0);

void tint_symbol_2(RWByteAddressBuffer buffer, uint offset, float value[4]) {
  float array_1[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      buffer.Store((offset + (i * 4u)), asuint(array_1[i]));
    }
  }
}

typedef float tint_symbol_4_ret[4];
tint_symbol_4_ret tint_symbol_4(ByteAddressBuffer buffer, uint offset) {
  float arr[4] = (float[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = asfloat(buffer.Load((offset + (i_1 * 4u))));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_2(tint_symbol_1, 0u, tint_symbol_4(tint_symbol, 0u));
  return;
}
