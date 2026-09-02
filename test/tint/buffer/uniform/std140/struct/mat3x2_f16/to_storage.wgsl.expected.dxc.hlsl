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
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

void v_1(uint offset, matrix<float16_t, 3, 2> obj) {
  s.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  s.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
}

matrix<float16_t, 3, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  return matrix<float16_t, 3, 2>(v_3, v_5, tint_bitcast_to_f16(u[(v_6 / 16u)][((v_6 & 15u) >> 2u)]));
}

void v_7(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v_1((offset + 4u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_8(uint start_byte_offset) {
  int v_9 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  matrix<float16_t, 3, 2> v_10 = v_2((4u + start_byte_offset));
  uint v_11 = (64u + start_byte_offset);
  S v_12 = {v_9, v_10, asint(u[(v_11 / 16u)][((v_11 & 15u) >> 2u)])};
  return v_12;
}

void v_13(uint offset, S obj[4]) {
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      S v_16 = obj[v_15];
      v_7((offset + (v_15 * 128u)), v_16);
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
}

typedef S ary_ret[4];
ary_ret v_17(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 4u)) {
        break;
      }
      S v_20 = v_8((start_byte_offset + (v_19 * 128u)));
      a[v_19] = v_20;
      {
        v_18 = (v_19 + 1u);
      }
    }
  }
  S v_21[4] = a;
  return v_21;
}

[numthreads(1, 1, 1)]
void f() {
  S v_22[4] = v_17(0u);
  v_13(0u, v_22);
  S v_23 = v_8(256u);
  v_7(128u, v_23);
  v_1(388u, v_2(260u));
  s.Store<vector<float16_t, 2> >(132u, tint_bitcast_to_f16(u[0u].z).yx);
}

