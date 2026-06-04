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
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = u[(v_3 / 16u)];
  float2 v_5 = asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint v_6 = (16u + start_byte_offset);
  uint4 v_7 = u[(v_6 / 16u)];
  return float3x2(v_2, v_5, asfloat((((((v_6 & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

S v_8(uint start_byte_offset) {
  int v_9 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float3x2 v_10 = v((8u + start_byte_offset));
  uint v_11 = (64u + start_byte_offset);
  S v_12 = {v_9, v_10, asint(u[(v_11 / 16u)][((v_11 & 15u) >> 2u)])};
  return v_12;
}

typedef S ary_ret[4];
ary_ret v_13(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      S v_16 = v_8((start_byte_offset + (v_15 * 128u)));
      a[v_15] = v_16;
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  S v_17[4] = a;
  return v_17;
}

[numthreads(1, 1, 1)]
void f() {
  S v_18[4] = v_13(0u);
  p = v_18;
  S v_19 = v_8(256u);
  p[1u] = v_19;
  p[3u].m = v(264u);
  p[1u].m[0u] = asfloat(u[1u].xy).yx;
}

