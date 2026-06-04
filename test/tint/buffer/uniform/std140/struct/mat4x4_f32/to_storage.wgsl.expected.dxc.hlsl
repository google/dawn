struct S {
  int before;
  float4x4 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[48];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float4x4 obj) {
  s.Store4((offset + 0u), asuint(obj[0u]));
  s.Store4((offset + 16u), asuint(obj[1u]));
  s.Store4((offset + 32u), asuint(obj[2u]));
  s.Store4((offset + 48u), asuint(obj[3u]));
}

float4x4 v_1(uint start_byte_offset) {
  return float4x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]), asfloat(u[((32u + start_byte_offset) / 16u)]), asfloat(u[((48u + start_byte_offset) / 16u)]));
}

void v_2(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v((offset + 16u), obj.m);
  s.Store((offset + 128u), asuint(obj.after));
}

S v_3(uint start_byte_offset) {
  int v_4 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float4x4 v_5 = v_1((16u + start_byte_offset));
  uint v_6 = (128u + start_byte_offset);
  S v_7 = {v_4, v_5, asint(u[(v_6 / 16u)][((v_6 & 15u) >> 2u)])};
  return v_7;
}

void v_8(uint offset, S obj[4]) {
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      S v_11 = obj[v_10];
      v_2((offset + (v_10 * 192u)), v_11);
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
}

typedef S ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      S v_15 = v_3((start_byte_offset + (v_14 * 192u)));
      a[v_14] = v_15;
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  S v_16[4] = a;
  return v_16;
}

[numthreads(1, 1, 1)]
void f() {
  S v_17[4] = v_12(0u);
  v_8(0u, v_17);
  S v_18 = v_3(384u);
  v_2(192u, v_18);
  v(592u, v_1(400u));
  s.Store4(208u, asuint(asfloat(u[2u]).ywxz));
}

