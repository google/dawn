
ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, matrix<float16_t, 4, 2> obj) {
  tint_symbol_1.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  tint_symbol_1.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  tint_symbol_1.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
  tint_symbol_1.Store<vector<float16_t, 2> >((offset + 12u), obj[3u]);
}

matrix<float16_t, 4, 2> v_1(uint offset) {
  return matrix<float16_t, 4, 2>(tint_symbol.Load<vector<float16_t, 2> >((offset + 0u)), tint_symbol.Load<vector<float16_t, 2> >((offset + 4u)), tint_symbol.Load<vector<float16_t, 2> >((offset + 8u)), tint_symbol.Load<vector<float16_t, 2> >((offset + 12u)));
}

[numthreads(1, 1, 1)]
void main() {
  v(0u, v_1(0u));
}

