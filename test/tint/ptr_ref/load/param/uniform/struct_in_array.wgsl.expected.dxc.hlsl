struct str {
  int4 i;
};

cbuffer cbuffer_S : register(b0, space0) {
  uint4 S[4];
};

str tint_symbol(uint4 buffer[4], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const str tint_symbol_2 = {asint(buffer[scalar_offset / 4])};
  return tint_symbol_2;
}

str func_S_X(uint pointer[1]) {
  return tint_symbol(S, (16u * pointer[0]));
}

[numthreads(1, 1, 1)]
void main() {
  const uint tint_symbol_3[1] = {2u};
  const str r = func_S_X(tint_symbol_3);
  return;
}
