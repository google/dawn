struct S {
  int before;
  float3x4 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float3x4 obj) {
  s.Store4((offset + 0u), asuint(obj[0u]));
  s.Store4((offset + 16u), asuint(obj[1u]));
  s.Store4((offset + 32u), asuint(obj[2u]));
}

float3x4 v_1(uint start_byte_offset) {
  float4 v_2 = asfloat(u[(start_byte_offset / 16u)]);
  float4 v_3 = asfloat(u[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_2, v_3, asfloat(u[((32u + start_byte_offset) / 16u)]));
}

void v_4(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v((offset + 16u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_5(uint start_byte_offset) {
  int v_6 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float3x4 v_7 = v_1((16u + start_byte_offset));
  S v_8 = {v_6, v_7, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_8;
}

void v_9(uint offset, S obj[4]) {
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      S v_12 = obj[v_11];
      v_4((offset + (v_11 * 128u)), v_12);
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
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
      S v_16 = v_5((start_byte_offset + (v_15 * 128u)));
      a[v_15] = v_16;
      {
        v_14 = (v_15 + 1u);
      }
      continue;
    }
  }
  S v_17[4] = a;
  return v_17;
}

[numthreads(1, 1, 1)]
void f() {
  S v_18[4] = v_13(0u);
  v_9(0u, v_18);
  S v_19 = v_5(256u);
  v_4(128u, v_19);
  v(400u, v_1(272u));
  s.Store4(144u, asuint(asfloat(u[2u]).ywxz));
}

