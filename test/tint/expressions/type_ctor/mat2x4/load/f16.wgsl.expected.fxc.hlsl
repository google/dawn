SKIP: FAILED

RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, matrix<float16_t, 2, 4> value) {
  tint_symbol.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  tint_symbol.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 4> m = matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
  tint_symbol_store(0u, matrix<float16_t, 2, 4>(m));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000166E0AB4590(3,44-52): error X3000: syntax error: unexpected token 'float16_t'
C:\src\dawn\Shader@0x00000166E0AB4590(4,3-19): error X3018: invalid subscript 'Store'

