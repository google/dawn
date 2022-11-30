SKIP: FAILED

ByteAddressBuffer tint_symbol : register(t0, space0);
RWByteAddressBuffer tint_symbol_1 : register(u1, space0);

void tint_symbol_2(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 4, 4> value) {
  buffer.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
  buffer.Store<vector<float16_t, 4> >((offset + 16u), value[2u]);
  buffer.Store<vector<float16_t, 4> >((offset + 24u), value[3u]);
}

matrix<float16_t, 4, 4> tint_symbol_4(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 4, 4>(buffer.Load<vector<float16_t, 4> >((offset + 0u)), buffer.Load<vector<float16_t, 4> >((offset + 8u)), buffer.Load<vector<float16_t, 4> >((offset + 16u)), buffer.Load<vector<float16_t, 4> >((offset + 24u)));
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_2(tint_symbol_1, 0u, tint_symbol_4(tint_symbol, 0u));
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x0000023AE8057740(4,68-76): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x0000023AE8057740(5,3-14): error X3018: invalid subscript 'Store'

