struct S {
  int before;
  matrix<float16_t, 4, 2> m;
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

void v_1(uint offset, matrix<float16_t, 4, 2> obj) {
  s.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  s.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
  s.Store<vector<float16_t, 2> >((offset + 12u), obj[3u]);
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  vector<float16_t, 2> v_7 = tint_bitcast_to_f16(u[(v_6 / 16u)][((v_6 & 15u) >> 2u)]);
  uint v_8 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_3, v_5, v_7, tint_bitcast_to_f16(u[(v_8 / 16u)][((v_8 & 15u) >> 2u)]));
}

void v_9(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_1((offset + 4u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_10(uint start_byte_offset) {
  int v_11 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 4, 2> v_12 = v_2((4u + start_byte_offset));
  uint v_13 = (64u + start_byte_offset);
  S v_14 = {v_11, v_12, asint(u[(v_13 / 16u)][((v_13 & 15u) >> 2u)])};
  return v_14;
}

void v_15(uint offset, S obj[4]) {
  {
    uint v_16 = 0u;
    v_16 = 0u;
    while(true) {
      uint v_17 = v_16;
      if ((v_17 >= 4u)) {
        break;
      }
      S v_18 = obj[v_17];
      v_9((offset + (v_17 * 128u)), v_18);
      {
        v_16 = (v_17 + 1u);
      }
    }
  }
}

typedef S ary_ret[4];
ary_ret v_19(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_20 = 0u;
    v_20 = 0u;
    while(true) {
      uint v_21 = v_20;
      if ((v_21 >= 4u)) {
        break;
      }
      S v_22 = v_10((start_byte_offset + (v_21 * 128u)));
      a[v_21] = v_22;
      {
        v_20 = (v_21 + 1u);
      }
    }
  }
  S v_23[4] = a;
  return v_23;
}

[numthreads(1, 1, 1)]
void f() {
  S v_24[4] = v_19(0u);
  v_15(0u, v_24);
  S v_25 = v_10(256u);
  v_9(128u, v_25);
  v_1(388u, v_2(260u));
  s.Store<vector<float16_t, 2> >(132u, tint_bitcast_to_f16(u[0u].z).yx);
}

