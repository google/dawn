RWByteAddressBuffer S : register(u0, space0);

void func_S_X(uint pointer[1]) {
  S.Store4((16u * pointer[0]), asuint((0.0f).xxxx));
}

[numthreads(1, 1, 1)]
void main() {
  const uint tint_symbol_1[1] = {1u};
  func_S_X(tint_symbol_1);
  return;
}
