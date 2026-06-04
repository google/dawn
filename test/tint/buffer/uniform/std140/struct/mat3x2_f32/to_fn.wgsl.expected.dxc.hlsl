struct S {
  int before;
  float3x2 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
void a(S a_1[4]) {
}

void b(S s) {
}

void c(float3x2 m) {
}

void d(float2 v) {
}

void e(float f_1) {
}

float3x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = u[(v_3 / 16u)];
  uint v_5 = (16u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)), asfloat(select((((v_3 & 15u) >> 2u) == 2u), v_4.zw, v_4.xy)), asfloat(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)));
}

S v_7(uint start_byte_offset) {
  int v_8 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float3x2 v_9 = v_1((8u + start_byte_offset));
  uint v_10 = (64u + start_byte_offset);
  S v_11 = {v_8, v_9, asint(u[(v_10 / 16u)][((v_10 & 15u) >> 2u)])};
  return v_11;
}

typedef S ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  S a_2[4] = (S[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      S v_15 = v_7((start_byte_offset + (v_14 * 128u)));
      a_2[v_14] = v_15;
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  S v_16[4] = a_2;
  return v_16;
}

[numthreads(1, 1, 1)]
void f() {
  S v_17[4] = v_12(0u);
  a(v_17);
  S v_18 = v_7(256u);
  b(v_18);
  c(v_1(264u));
  d(asfloat(u[1u].xy).yx);
  e(asfloat(u[1u].xy).yx.x);
}

