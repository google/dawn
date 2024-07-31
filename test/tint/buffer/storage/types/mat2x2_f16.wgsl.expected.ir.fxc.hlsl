SKIP: FAILED


ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, matrix<float16_t, 2, 2> obj) {
  tint_symbol_1.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  tint_symbol_1.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
}

matrix<float16_t, 2, 2> v_1(uint offset) {
  vector<float16_t, 2> v_2 = tint_symbol.Load<vector<float16_t, 2> >((offset + 0u));
  return matrix<float16_t, 2, 2>(v_2, tint_symbol.Load<vector<float16_t, 2> >((offset + 4u)));
}

[numthreads(1, 1, 1)]
void main() {
  v(0u, v_1(0u));
}

FXC validation failure:
c:\src\dawn\Shader@0x0000026E3C25F000(4,28-36): error X3000: syntax error: unexpected token 'float16_t'
c:\src\dawn\Shader@0x0000026E3C25F000(5,3-21): error X3018: invalid subscript 'Store'

