
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
float a(float4x2 a_1[4]) {
  return a_1[0u][0u].x;
}

float b(float4x2 m) {
  return m[0u].x;
}

float c(float2 v) {
  return v.x;
}

float d(float f_1) {
  return f_1;
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

typedef float4x2 ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  float4x2 a_2[4] = (float4x2[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a_2[v_14] = v_1((start_byte_offset + (v_14 * 32u)));
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  float4x2 v_15[4] = a_2;
  return v_15;
}

[numthreads(1, 1, 1)]
void f() {
  float4x2 v_16[4] = v_12(0u);
  float v_17 = a(v_16);
  float v_18 = (v_17 + b(v_1(32u)));
  float v_19 = (v_18 + c(asfloat(u[2u].xy).yx));
  s.Store(0u, asuint((v_19 + d(asfloat(u[2u].xy).yx.x))));
}

