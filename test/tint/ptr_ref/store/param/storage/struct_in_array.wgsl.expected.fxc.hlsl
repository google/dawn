struct str {
  int i;
};

RWByteAddressBuffer S : register(u0, space0);

void tint_symbol(RWByteAddressBuffer buffer, uint offset, str value) {
  buffer.Store((offset + 0u), asuint(value.i));
}

void func_S_X(uint pointer[1]) {
  const str tint_symbol_2 = (str)0;
  tint_symbol(S, (4u * pointer[0]), tint_symbol_2);
}

[numthreads(1, 1, 1)]
void main() {
  const uint tint_symbol_3[1] = {2u};
  func_S_X(tint_symbol_3);
  return;
}
