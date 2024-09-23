struct S {
  int before;
  float2x3 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;
float2x3 v(uint start_byte_offset) {
  float3 v_1 = asfloat(u[(start_byte_offset / 16u)].xyz);
  return float2x3(v_1, asfloat(u[((16u + start_byte_offset) / 16u)].xyz));
}

S v_2(uint start_byte_offset) {
  int v_3 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float2x3 v_4 = v((16u + start_byte_offset));
  S v_5 = {v_3, v_4, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
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
      S v_9 = v_2((start_byte_offset + (v_8 * 128u)));
      a[v_8] = v_9;
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  S v_10[4] = a;
  return v_10;
}

[numthreads(1, 1, 1)]
void f() {
  S v_11[4] = v_6(0u);
  p = v_11;
  S v_12 = v_2(256u);
  p[int(1)] = v_12;
  p[int(3)].m = v(272u);
  p[int(1)].m[int(0)] = asfloat(u[2u].xyz).zxy;
}

