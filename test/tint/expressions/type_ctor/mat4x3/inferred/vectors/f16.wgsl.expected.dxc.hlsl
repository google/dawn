
static matrix<float16_t, 4, 3> m = matrix<float16_t, 4, 3>(vector<float16_t, 3>(float16_t(0.0h), float16_t(1.0h), float16_t(2.0h)), vector<float16_t, 3>(float16_t(3.0h), float16_t(4.0h), float16_t(5.0h)), vector<float16_t, 3>(float16_t(6.0h), float16_t(7.0h), float16_t(8.0h)), vector<float16_t, 3>(float16_t(9.0h), float16_t(10.0h), float16_t(11.0h)));
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, matrix<float16_t, 4, 3> obj) {
  v.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  v.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  v.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
  v.Store<vector<float16_t, 3> >((offset + 24u), obj[3u]);
}

[numthreads(1, 1, 1)]
void f() {
  v_1(0u, m);
}

