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
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

void v_2(uint offset, matrix<float16_t, 4, 2> obj) {
  s.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  s.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
  s.Store<vector<float16_t, 2> >((offset + 12u), obj[3u]);
}

matrix<float16_t, 4, 2> v_3(uint start_byte_offset) {
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_5 = (4u + start_byte_offset);
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(u[(v_5 / 16u)][((v_5 & 15u) >> 2u)]);
  uint v_7 = (8u + start_byte_offset);
  vector<float16_t, 2> v_8 = tint_bitcast_to_f16(u[(v_7 / 16u)][((v_7 & 15u) >> 2u)]);
  uint v_9 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_4, v_6, v_8, tint_bitcast_to_f16(u[(v_9 / 16u)][((v_9 & 15u) >> 2u)]));
}

void v_10(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_2((offset + 4u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_11(uint start_byte_offset) {
  int v_12 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 4, 2> v_13 = v_3((4u + start_byte_offset));
  uint v_14 = (64u + start_byte_offset);
  S v_15 = {v_12, v_13, asint(u[(v_14 / 16u)][((v_14 & 15u) >> 2u)])};
  return v_15;
}

void v_16(uint offset, S obj[4]) {
  {
    uint v_17 = 0u;
    v_17 = 0u;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      S v_19 = obj[v_18];
      v_10((offset + (v_18 * 128u)), v_19);
      {
        v_17 = (v_18 + 1u);
      }
    }
  }
}

typedef S ary_ret[4];
ary_ret v_20(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_21 = 0u;
    v_21 = 0u;
    while(true) {
      uint v_22 = v_21;
      if ((v_22 >= 4u)) {
        break;
      }
      S v_23 = v_11((start_byte_offset + (v_22 * 128u)));
      a[v_22] = v_23;
      {
        v_21 = (v_22 + 1u);
      }
    }
  }
  S v_24[4] = a;
  return v_24;
}

[numthreads(1, 1, 1)]
void f() {
  S v_25[4] = v_20(0u);
  v_16(0u, v_25);
  S v_26 = v_11(256u);
  v_10(128u, v_26);
  v_2(388u, v_3(260u));
  s.Store<vector<float16_t, 2> >(132u, tint_bitcast_to_f16(u[0u].z).yx);
}

