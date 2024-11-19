struct Inner {
  float16_t scalar_f16;
  vector<float16_t, 3> vec3_f16;
  matrix<float16_t, 2, 4> mat2x4_f16;
};

struct S {
  Inner inner;
};


ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, matrix<float16_t, 2, 4> obj) {
  tint_symbol_1.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  tint_symbol_1.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

void v_1(uint offset, Inner obj) {
  tint_symbol_1.Store<float16_t>((offset + 0u), obj.scalar_f16);
  tint_symbol_1.Store<vector<float16_t, 3> >((offset + 8u), obj.vec3_f16);
  v((offset + 16u), obj.mat2x4_f16);
}

void v_2(uint offset, S obj) {
  Inner v_3 = obj.inner;
  v_1((offset + 0u), v_3);
}

matrix<float16_t, 2, 4> v_4(uint offset) {
  return matrix<float16_t, 2, 4>(tint_symbol.Load<vector<float16_t, 4> >((offset + 0u)), tint_symbol.Load<vector<float16_t, 4> >((offset + 8u)));
}

Inner v_5(uint offset) {
  float16_t v_6 = tint_symbol.Load<float16_t>((offset + 0u));
  vector<float16_t, 3> v_7 = tint_symbol.Load<vector<float16_t, 3> >((offset + 8u));
  Inner v_8 = {v_6, v_7, v_4((offset + 16u))};
  return v_8;
}

S v_9(uint offset) {
  Inner v_10 = v_5((offset + 0u));
  S v_11 = {v_10};
  return v_11;
}

[numthreads(1, 1, 1)]
void main() {
  S t = v_9(0u);
  v_2(0u, t);
}

