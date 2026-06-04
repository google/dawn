struct S {
  int before;
  float3x3 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;
float3x3 v(uint start_byte_offset) {
  return float3x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz), asfloat(u[((32u + start_byte_offset) / 16u)].xyz));
}

S v_1(uint start_byte_offset) {
  int v_2 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float3x3 v_3 = v((16u + start_byte_offset));
  uint v_4 = (64u + start_byte_offset);
  S v_5 = {v_2, v_3, asint(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)])};
  return v_5;
}

typedef S ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      S v_9 = v_1((start_byte_offset + (v_8 * 128u)));
      a[v_8] = v_9;
      {
        v_7 = (v_8 + 1u);
      }
    }
  }
  S v_10[4] = a;
  return v_10;
}

[numthreads(1, 1, 1)]
void f() {
  S v_11[4] = v_6(0u);
  p = v_11;
  S v_12 = v_1(256u);
  p[1u] = v_12;
  p[3u].m = v(272u);
  p[1u].m[0u] = asfloat(u[2u].xyz).zxy;
}

