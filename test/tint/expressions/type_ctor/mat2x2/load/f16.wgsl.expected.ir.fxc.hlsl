SKIP: FAILED


RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, matrix<float16_t, 2, 2> obj) {
  tint_symbol.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  tint_symbol.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 2> m = matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
  v(0u, matrix<float16_t, 2, 2>(m));
}

FXC validation failure:
c:\src\dawn\Shader@0x000002BA7DADD070(3,28-36): error X3000: syntax error: unexpected token 'float16_t'
c:\src\dawn\Shader@0x000002BA7DADD070(4,3-19): error X3018: invalid subscript 'Store'

