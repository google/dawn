struct S {
  int before;
  float4x2 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
void a(S a_1[4]) {
}

void b(S s) {
}

void c(float4x2 m) {
}

void d(float2 v) {
}

void e(float f_1) {
}

float4x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  float2 v_6 = asfloat((((((v_4 & 15u) >> 2u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = u[(v_7 / 16u)];
  float2 v_9 = asfloat((((((v_7 & 15u) >> 2u) == 2u)) ? (v_8.zw) : (v_8.xy)));
  uint v_10 = (24u + start_byte_offset);
  uint4 v_11 = u[(v_10 / 16u)];
  return float4x2(v_3, v_6, v_9, asfloat((((((v_10 & 15u) >> 2u) == 2u)) ? (v_11.zw) : (v_11.xy))));
}

S v_12(uint start_byte_offset) {
  int v_13 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float4x2 v_14 = v_1((8u + start_byte_offset));
  uint v_15 = (64u + start_byte_offset);
  S v_16 = {v_13, v_14, asint(u[(v_15 / 16u)][((v_15 & 15u) >> 2u)])};
  return v_16;
}

typedef S ary_ret[4];
ary_ret v_17(uint start_byte_offset) {
  S a_2[4] = (S[4])0;
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 4u)) {
        break;
      }
      S v_20 = v_12((start_byte_offset + (v_19 * 128u)));
      a_2[v_19] = v_20;
      {
        v_18 = (v_19 + 1u);
      }
    }
  }
  S v_21[4] = a_2;
  return v_21;
}

[numthreads(1, 1, 1)]
void f() {
  S v_22[4] = v_17(0u);
  a(v_22);
  S v_23 = v_12(256u);
  b(v_23);
  c(v_1(264u));
  d(asfloat(u[1u].xy).yx);
  e(asfloat(u[1u].xy).yx.x);
}

