SKIP: FAILED


ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1.Store<float16_t>(0u, tint_symbol.Load<float16_t>(0u));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000018D8C9B3B30(6,3-21): error X3018: invalid subscript 'Store'

