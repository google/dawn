struct S {
  int before;
  float3x4 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;
float3x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(u[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(u[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_1, v_2, asfloat(u[((32u + start_byte_offset) / 16u)]));
}

S v_3(uint start_byte_offset) {
  int v_4 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float3x4 v_5 = v((16u + start_byte_offset));
  S v_6 = {v_4, v_5, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_6;
}

typedef S ary_ret[4];
ary_ret v_7(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      S v_10 = v_3((start_byte_offset + (v_9 * 128u)));
      a[v_9] = v_10;
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  S v_11[4] = a;
  return v_11;
}

[numthreads(1, 1, 1)]
void f() {
  S v_12[4] = v_7(0u);
  p = v_12;
  S v_13 = v_3(256u);
  p[int(1)] = v_13;
  p[int(3)].m = v(272u);
  p[int(1)].m[int(0)] = asfloat(u[2u]).ywxz;
}

