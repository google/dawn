struct S {
  int before;
  float4x3 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[48];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float4x3 obj) {
  s.Store3((offset + 0u), asuint(obj[0u]));
  s.Store3((offset + 16u), asuint(obj[1u]));
  s.Store3((offset + 32u), asuint(obj[2u]));
  s.Store3((offset + 48u), asuint(obj[3u]));
}

float4x3 v_1(uint start_byte_offset) {
  float3 v_2 = asfloat(u[(start_byte_offset / 16u)].xyz);
  float3 v_3 = asfloat(u[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_4 = asfloat(u[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_2, v_3, v_4, asfloat(u[((48u + start_byte_offset) / 16u)].xyz));
}

void v_5(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v((offset + 16u), obj.m);
  s.Store((offset + 128u), asuint(obj.after));
}

S v_6(uint start_byte_offset) {
  int v_7 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float4x3 v_8 = v_1((16u + start_byte_offset));
  S v_9 = {v_7, v_8, asint(u[((128u + start_byte_offset) / 16u)][(((128u + start_byte_offset) % 16u) / 4u)])};
  return v_9;
}

void v_10(uint offset, S obj[4]) {
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      S v_13 = obj[v_12];
      v_5((offset + (v_12 * 192u)), v_13);
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
}

typedef S ary_ret[4];
ary_ret v_14(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      S v_17 = v_6((start_byte_offset + (v_16 * 192u)));
      a[v_16] = v_17;
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
  S v_18[4] = a;
  return v_18;
}

[numthreads(1, 1, 1)]
void f() {
  S v_19[4] = v_14(0u);
  v_10(0u, v_19);
  S v_20 = v_6(384u);
  v_5(192u, v_20);
  v(592u, v_1(400u));
  s.Store3(208u, asuint(asfloat(u[2u].xyz).zxy));
}

