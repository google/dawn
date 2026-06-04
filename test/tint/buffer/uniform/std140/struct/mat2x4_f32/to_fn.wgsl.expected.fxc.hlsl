struct S {
  int before;
  float2x4 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
void a(S a_1[4]) {
}

void b(S s) {
}

void c(float2x4 m) {
}

void d(float4 v) {
}

void e(float f_1) {
}

float2x4 v_1(uint start_byte_offset) {
  return float2x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]));
}

S v_2(uint start_byte_offset) {
  int v_3 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float2x4 v_4 = v_1((16u + start_byte_offset));
  uint v_5 = (64u + start_byte_offset);
  S v_6 = {v_3, v_4, asint(u[(v_5 / 16u)][((v_5 & 15u) >> 2u)])};
  return v_6;
}

typedef S ary_ret[4];
ary_ret v_7(uint start_byte_offset) {
  S a_2[4] = (S[4])0;
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      S v_10 = v_2((start_byte_offset + (v_9 * 128u)));
      a_2[v_9] = v_10;
      {
        v_8 = (v_9 + 1u);
      }
    }
  }
  S v_11[4] = a_2;
  return v_11;
}

[numthreads(1, 1, 1)]
void f() {
  S v_12[4] = v_7(0u);
  a(v_12);
  S v_13 = v_2(256u);
  b(v_13);
  c(v_1(272u));
  d(asfloat(u[2u]).ywxz);
  e(asfloat(u[2u]).ywxz.x);
}

