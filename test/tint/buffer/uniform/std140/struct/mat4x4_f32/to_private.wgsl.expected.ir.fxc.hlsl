struct S {
  int before;
  float4x4 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[48];
};
static S p[4] = (S[4])0;
float4x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(u[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(u[((16u + start_byte_offset) / 16u)]);
  float4 v_3 = asfloat(u[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_1, v_2, v_3, asfloat(u[((48u + start_byte_offset) / 16u)]));
}

S v_4(uint start_byte_offset) {
  int v_5 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float4x4 v_6 = v((16u + start_byte_offset));
  S v_7 = {v_5, v_6, asint(u[((128u + start_byte_offset) / 16u)][(((128u + start_byte_offset) % 16u) / 4u)])};
  return v_7;
}

typedef S ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      S v_11 = v_4((start_byte_offset + (v_10 * 192u)));
      a[v_10] = v_11;
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  S v_12[4] = a;
  return v_12;
}

[numthreads(1, 1, 1)]
void f() {
  S v_13[4] = v_8(0u);
  p = v_13;
  S v_14 = v_4(384u);
  p[int(1)] = v_14;
  p[int(3)].m = v(400u);
  p[int(1)].m[int(0)] = asfloat(u[2u]).ywxz;
}

