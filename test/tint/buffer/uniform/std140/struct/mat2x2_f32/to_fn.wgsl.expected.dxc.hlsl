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
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = u[(v_3 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)), asfloat(select((((v_3 & 15u) >> 2u) == 2u), v_4.zw, v_4.xy)));
}

S v_5(uint start_byte_offset) {
  int v_6 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float2x2 v_7 = v_1((8u + start_byte_offset));
  uint v_8 = (64u + start_byte_offset);
  S v_9 = {v_6, v_7, asint(u[(v_8 / 16u)][((v_8 & 15u) >> 2u)])};
  return v_9;
}

typedef S ary_ret[4];
ary_ret v_10(uint start_byte_offset) {
  S a_2[4] = (S[4])0;
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      S v_13 = v_5((start_byte_offset + (v_12 * 128u)));
      a_2[v_12] = v_13;
      {
        v_11 = (v_12 + 1u);
      }
    }
  }
  S v_14[4] = a_2;
  return v_14;
}

[numthreads(1, 1, 1)]
void f() {
  S v_15[4] = v_10(0u);
  a(v_15);
  S v_16 = v_5(256u);
  b(v_16);
  c(v_1(264u));
  d(asfloat(u[1u].xy).yx);
  e(asfloat(u[1u].xy).yx.x);
}

