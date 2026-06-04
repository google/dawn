
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
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = u[(v_3 / 16u)];
  uint v_5 = (16u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  uint v_7 = (24u + start_byte_offset);
  uint4 v_8 = u[(v_7 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)), asfloat(select((((v_3 & 15u) >> 2u) == 2u), v_4.zw, v_4.xy)), asfloat(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)), asfloat(select((((v_7 & 15u) >> 2u) == 2u), v_8.zw, v_8.xy)));
}

[numthreads(1, 1, 1)]
void f() {
  a(v_1(0u));
  b(asfloat(u[0u].zw));
  b(asfloat(u[0u].zw).yx);
  c(asfloat(u[0u].z));
  c(asfloat(u[0u].zw).yx.x);
}

