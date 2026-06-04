struct S {
  int before;
  matrix<float16_t, 4, 4> m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
RWByteAddressBuffer s : register(u1);
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

void v_1(uint offset, matrix<float16_t, 4, 4> obj) {
  s.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  s.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
  s.Store<vector<float16_t, 4> >((offset + 24u), obj[3u]);
}

matrix<float16_t, 4, 4> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_4 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy));
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  vector<float16_t, 4> v_7 = tint_bitcast_to_f16(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy));
  uint v_8 = (16u + start_byte_offset);
  uint4 v_9 = u[(v_8 / 16u)];
  vector<float16_t, 4> v_10 = tint_bitcast_to_f16(select((((v_8 & 15u) >> 2u) == 2u), v_9.zw, v_9.xy));
  uint v_11 = (24u + start_byte_offset);
  uint4 v_12 = u[(v_11 / 16u)];
  return matrix<float16_t, 4, 4>(v_4, v_7, v_10, tint_bitcast_to_f16(select((((v_11 & 15u) >> 2u) == 2u), v_12.zw, v_12.xy)));
}

void v_13(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_1((offset + 8u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_14(uint start_byte_offset) {
  int v_15 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 4, 4> v_16 = v_2((8u + start_byte_offset));
  uint v_17 = (64u + start_byte_offset);
  S v_18 = {v_15, v_16, asint(u[(v_17 / 16u)][((v_17 & 15u) >> 2u)])};
  return v_18;
}

void v_19(uint offset, S obj[4]) {
  {
    uint v_20 = 0u;
    v_20 = 0u;
    while(true) {
      uint v_21 = v_20;
      if ((v_21 >= 4u)) {
        break;
      }
      S v_22 = obj[v_21];
      v_13((offset + (v_21 * 128u)), v_22);
      {
        v_20 = (v_21 + 1u);
      }
    }
  }
}

typedef S ary_ret[4];
ary_ret v_23(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_24 = 0u;
    v_24 = 0u;
    while(true) {
      uint v_25 = v_24;
      if ((v_25 >= 4u)) {
        break;
      }
      S v_26 = v_14((start_byte_offset + (v_25 * 128u)));
      a[v_25] = v_26;
      {
        v_24 = (v_25 + 1u);
      }
    }
  }
  S v_27[4] = a;
  return v_27;
}

[numthreads(1, 1, 1)]
void f() {
  S v_28[4] = v_23(0u);
  v_19(0u, v_28);
  S v_29 = v_14(256u);
  v_13(128u, v_29);
  v_1(392u, v_2(264u));
  s.Store<vector<float16_t, 4> >(136u, tint_bitcast_to_f16(u[1u].xy).ywxz);
}

