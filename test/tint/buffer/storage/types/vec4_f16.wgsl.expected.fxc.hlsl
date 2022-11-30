SKIP: FAILED

ByteAddressBuffer tint_symbol : register(t0, space0);
RWByteAddressBuffer tint_symbol_1 : register(u1, space0);

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1.Store<vector<float16_t, 4> >(0u, tint_symbol.Load<vector<float16_t, 4> >(0u));
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x000001D5DD394680(6,3-21): error X3018: invalid subscript 'Store'

