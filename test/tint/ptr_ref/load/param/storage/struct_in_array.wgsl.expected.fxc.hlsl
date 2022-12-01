struct str {
  int i;
};

ByteAddressBuffer S : register(t0, space0);

str tint_symbol(ByteAddressBuffer buffer, uint offset) {
  const str tint_symbol_2 = {asint(buffer.Load((offset + 0u)))};
  return tint_symbol_2;
}

str func_S_X(uint pointer[1]) {
  return tint_symbol(S, (4u * pointer[0]));
}

[numthreads(1, 1, 1)]
void main() {
  const uint tint_symbol_3[1] = {2u};
  const str r = func_S_X(tint_symbol_3);
  return;
}
