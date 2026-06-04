struct S {
  int before;
  matrix<float16_t, 3, 2> m;
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

void v_2(uint offset, matrix<float16_t, 3, 2> obj) {
  s.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  s.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
}

matrix<float16_t, 3, 2> v_3(uint start_byte_offset) {
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_5 = (4u + start_byte_offset);
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(u[(v_5 / 16u)][((v_5 & 15u) >> 2u)]);
  uint v_7 = (8u + start_byte_offset);
  return matrix<float16_t, 3, 2>(v_4, v_6, tint_bitcast_to_f16(u[(v_7 / 16u)][((v_7 & 15u) >> 2u)]));
}

void v_8(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_2((offset + 4u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_9(uint start_byte_offset) {
  int v_10 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 3, 2> v_11 = v_3((4u + start_byte_offset));
  uint v_12 = (64u + start_byte_offset);
  S v_13 = {v_10, v_11, asint(u[(v_12 / 16u)][((v_12 & 15u) >> 2u)])};
  return v_13;
}

void v_14(uint offset, S obj[4]) {
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      S v_17 = obj[v_16];
      v_8((offset + (v_16 * 128u)), v_17);
      {
        v_15 = (v_16 + 1u);
      }
    }
  }
}

typedef S ary_ret[4];
ary_ret v_18(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_19 = 0u;
    v_19 = 0u;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      S v_21 = v_9((start_byte_offset + (v_20 * 128u)));
      a[v_20] = v_21;
      {
        v_19 = (v_20 + 1u);
      }
    }
  }
  S v_22[4] = a;
  return v_22;
}

[numthreads(1, 1, 1)]
void f() {
  S v_23[4] = v_18(0u);
  v_14(0u, v_23);
  S v_24 = v_9(256u);
  v_8(128u, v_24);
  v_2(388u, v_3(260u));
  s.Store<vector<float16_t, 2> >(132u, tint_bitcast_to_f16(u[0u].z).yx);
}

