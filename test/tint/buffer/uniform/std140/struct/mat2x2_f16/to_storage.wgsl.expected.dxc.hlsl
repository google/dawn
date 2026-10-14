struct S {
  int before;
  matrix<float16_t, 2, 2> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

void v_1(uint offset, matrix<float16_t, 2, 2> obj) {
  s.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
}

matrix<float16_t, 2, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_3, tint_bitcast_to_f16(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)]));
}

void v_5(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_1((offset + 4u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_6(uint start_byte_offset) {
  int v_7 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 2, 2> v_8 = v_2((4u + start_byte_offset));
  uint v_9 = (64u + start_byte_offset);
  S v_10 = {v_7, v_8, asint(u[(v_9 / 16u)][((v_9 & 15u) >> 2u)])};
  return v_10;
}

void v_11(uint offset, S obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_12 = idx;
      if ((v_12 >= 4u)) {
        break;
      }
      S v_13 = obj[v_12];
      v_5((offset + (v_12 * 128u)), v_13);
      {
        idx = (idx + 1u);
      }
    }
  }
}

typedef S ary_ret[4];
ary_ret v_14(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_15 = idx;
      if ((v_15 >= 4u)) {
        break;
      }
      S v_16 = v_6((start_byte_offset + (v_15 * 128u)));
      a[v_15] = v_16;
      {
        idx = (idx + 1u);
      }
    }
  }
  S v_17[4] = a;
  return v_17;
}

[numthreads(1, 1, 1)]
void f() {
  S v_18[4] = v_14(0u);
  v_11(0u, v_18);
  S v_19 = v_6(256u);
  v_5(128u, v_19);
  v_1(388u, v_2(260u));
  s.Store<vector<float16_t, 2> >(132u, tint_bitcast_to_f16(u[0u].z).yx);
}

