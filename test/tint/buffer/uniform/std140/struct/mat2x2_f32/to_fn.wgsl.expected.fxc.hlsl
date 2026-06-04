struct S {
  int before;
  float2x2 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
void a(S a_1[4]) {
}

void b(S s) {
}

void c(float2x2 m) {
}

void d(float2 v) {
}

void e(float f_1) {
}

float2x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  return float2x2(v_3, asfloat((((((v_4 & 15u) >> 2u) == 2u)) ? (v_5.zw) : (v_5.xy))));
}

S v_6(uint start_byte_offset) {
  int v_7 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float2x2 v_8 = v_1((8u + start_byte_offset));
  uint v_9 = (64u + start_byte_offset);
  S v_10 = {v_7, v_8, asint(u[(v_9 / 16u)][((v_9 & 15u) >> 2u)])};
  return v_10;
}

typedef S ary_ret[4];
ary_ret v_11(uint start_byte_offset) {
  S a_2[4] = (S[4])0;
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      S v_14 = v_6((start_byte_offset + (v_13 * 128u)));
      a_2[v_13] = v_14;
      {
        v_12 = (v_13 + 1u);
      }
    }
  }
  S v_15[4] = a_2;
  return v_15;
}

[numthreads(1, 1, 1)]
void f() {
  S v_16[4] = v_11(0u);
  a(v_16);
  S v_17 = v_6(256u);
  b(v_17);
  c(v_1(264u));
  d(asfloat(u[1u].xy).yx);
  e(asfloat(u[1u].xy).yx.x);
}

