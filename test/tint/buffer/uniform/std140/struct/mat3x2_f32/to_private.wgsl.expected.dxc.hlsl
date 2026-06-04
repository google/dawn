struct S {
  int before;
  float3x2 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = u[(v_2 / 16u)];
  uint v_4 = (16u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)), asfloat(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)));
}

S v_6(uint start_byte_offset) {
  int v_7 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float3x2 v_8 = v((8u + start_byte_offset));
  uint v_9 = (64u + start_byte_offset);
  S v_10 = {v_7, v_8, asint(u[(v_9 / 16u)][((v_9 & 15u) >> 2u)])};
  return v_10;
}

typedef S ary_ret[4];
ary_ret v_11(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      S v_14 = v_6((start_byte_offset + (v_13 * 128u)));
      a[v_13] = v_14;
      {
        v_12 = (v_13 + 1u);
      }
    }
  }
  S v_15[4] = a;
  return v_15;
}

[numthreads(1, 1, 1)]
void f() {
  S v_16[4] = v_11(0u);
  p = v_16;
  S v_17 = v_6(256u);
  p[1u] = v_17;
  p[3u].m = v(264u);
  p[1u].m[0u] = asfloat(u[1u].xy).yx;
}

