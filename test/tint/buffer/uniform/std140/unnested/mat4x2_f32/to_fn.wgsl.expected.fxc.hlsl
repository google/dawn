
cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
void a(float4x2 m) {
}

void b(float2 v) {
}

void c(float f_1) {
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

[numthreads(1, 1, 1)]
void f() {
  a(v_1(0u));
  b(asfloat(u[0u].zw));
  b(asfloat(u[0u].zw).yx);
  c(asfloat(u[0u].z));
  c(asfloat(u[0u].zw).yx.x);
}

