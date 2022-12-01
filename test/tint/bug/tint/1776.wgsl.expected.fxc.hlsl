struct S {
  float4 a;
  int b;
};

ByteAddressBuffer sb : register(t0, space0);

S tint_symbol(ByteAddressBuffer buffer, uint offset) {
  const S tint_symbol_3 = {asfloat(buffer.Load4((offset + 0u))), asint(buffer.Load((offset + 16u)))};
  return tint_symbol_3;
}

[numthreads(1, 1, 1)]
void main() {
  const S x = tint_symbol(sb, 32u);
  return;
}
