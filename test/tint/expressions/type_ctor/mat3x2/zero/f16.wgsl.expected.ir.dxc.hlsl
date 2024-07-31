
static matrix<float16_t, 3, 2> m = matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, matrix<float16_t, 3, 2> obj) {
  tint_symbol.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  tint_symbol.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  tint_symbol.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, m);
}

