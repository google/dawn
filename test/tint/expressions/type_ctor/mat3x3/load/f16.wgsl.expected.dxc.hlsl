
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, matrix<float16_t, 3, 3> obj) {
  v.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  v.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  v.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 3, 3> m = matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
  v_1(0u, matrix<float16_t, 3, 3>(m));
}

