struct S {
  int before;
  matrix<float16_t, 2, 3> m;
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

void v_1(uint offset, matrix<float16_t, 2, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
}

matrix<float16_t, 2, 3> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_4 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)).xyz;
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  return matrix<float16_t, 2, 3>(v_4, tint_bitcast_to_f16(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)).xyz);
}

void v_7(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_1((offset + 8u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_8(uint start_byte_offset) {
  int v_9 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 2, 3> v_10 = v_2((8u + start_byte_offset));
  uint v_11 = (64u + start_byte_offset);
  S v_12 = {v_9, v_10, asint(u[(v_11 / 16u)][((v_11 & 15u) >> 2u)])};
  return v_12;
}

void v_13(uint offset, S obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_14 = idx;
      if ((v_14 >= 4u)) {
        break;
      }
      S v_15 = obj[v_14];
      v_7((offset + (v_14 * 128u)), v_15);
      {
        idx = (idx + 1u);
      }
    }
  }
}

typedef S ary_ret[4];
ary_ret v_16(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_17 = idx;
      if ((v_17 >= 4u)) {
        break;
      }
      S v_18 = v_8((start_byte_offset + (v_17 * 128u)));
      a[v_17] = v_18;
      {
        idx = (idx + 1u);
      }
    }
  }
  S v_19[4] = a;
  return v_19;
}

[numthreads(1, 1, 1)]
void f() {
  S v_20[4] = v_16(0u);
  v_13(0u, v_20);
  S v_21 = v_8(256u);
  v_7(128u, v_21);
  v_1(392u, v_2(264u));
  s.Store<vector<float16_t, 3> >(136u, tint_bitcast_to_f16(u[1u].xy).xyz.zxy);
}

