struct S {
  int before;
  float4x2 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float4x2 obj) {
  s.Store2((offset + 0u), asuint(obj[0u]));
  s.Store2((offset + 8u), asuint(obj[1u]));
  s.Store2((offset + 16u), asuint(obj[2u]));
  s.Store2((offset + 24u), asuint(obj[3u]));
}

float4x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  float2 v_6 = asfloat((((((v_4 & 15u) >> 2u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = u[(v_7 / 16u)];
  float2 v_9 = asfloat((((((v_7 & 15u) >> 2u) == 2u)) ? (v_8.zw) : (v_8.xy)));
  uint v_10 = (24u + start_byte_offset);
  uint4 v_11 = u[(v_10 / 16u)];
  return float4x2(v_3, v_6, v_9, asfloat((((((v_10 & 15u) >> 2u) == 2u)) ? (v_11.zw) : (v_11.xy))));
}

void v_12(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v((offset + 8u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_13(uint start_byte_offset) {
  int v_14 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float4x2 v_15 = v_1((8u + start_byte_offset));
  uint v_16 = (64u + start_byte_offset);
  S v_17 = {v_14, v_15, asint(u[(v_16 / 16u)][((v_16 & 15u) >> 2u)])};
  return v_17;
}

void v_18(uint offset, S obj[4]) {
  {
    uint v_19 = 0u;
    v_19 = 0u;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      S v_21 = obj[v_20];
      v_12((offset + (v_20 * 128u)), v_21);
      {
        v_19 = (v_20 + 1u);
      }
    }
  }
}

typedef S ary_ret[4];
ary_ret v_22(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_23 = 0u;
    v_23 = 0u;
    while(true) {
      uint v_24 = v_23;
      if ((v_24 >= 4u)) {
        break;
      }
      S v_25 = v_13((start_byte_offset + (v_24 * 128u)));
      a[v_24] = v_25;
      {
        v_23 = (v_24 + 1u);
      }
    }
  }
  S v_26[4] = a;
  return v_26;
}

[numthreads(1, 1, 1)]
void f() {
  S v_27[4] = v_22(0u);
  v_18(0u, v_27);
  S v_28 = v_13(256u);
  v_12(128u, v_28);
  v(392u, v_1(264u));
  s.Store2(136u, asuint(asfloat(u[1u].xy).yx));
}

