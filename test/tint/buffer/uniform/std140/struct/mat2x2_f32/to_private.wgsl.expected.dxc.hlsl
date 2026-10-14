struct S {
  int before;
  float2x2 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;
float2x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = u[(v_2 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)));
}

S v_4(uint start_byte_offset) {
  int v_5 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float2x2 v_6 = v((8u + start_byte_offset));
  uint v_7 = (64u + start_byte_offset);
  S v_8 = {v_5, v_6, asint(u[(v_7 / 16u)][((v_7 & 15u) >> 2u)])};
  return v_8;
}

typedef S ary_ret[4];
ary_ret v_9(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_10 = idx;
      if ((v_10 >= 4u)) {
        break;
      }
      S v_11 = v_4((start_byte_offset + (v_10 * 128u)));
      a[v_10] = v_11;
      {
        idx = (idx + 1u);
      }
    }
  }
  S v_12[4] = a;
  return v_12;
}

[numthreads(1, 1, 1)]
void f() {
  S v_13[4] = v_9(0u);
  p = v_13;
  S v_14 = v_4(256u);
  p[int(1)] = v_14;
  p[int(3)].m = v(264u);
  p[int(1)].m[int(0)] = asfloat(u[1u].xy).yx;
}

