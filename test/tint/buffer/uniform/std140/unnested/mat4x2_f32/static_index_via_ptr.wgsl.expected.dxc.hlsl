
cbuffer cbuffer_m : register(b0) {
  uint4 m[2];
};
float4x2 v(uint start_byte_offset) {
  uint4 v_1 = m[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = m[(v_2 / 16u)];
  uint v_4 = (16u + start_byte_offset);
  uint4 v_5 = m[(v_4 / 16u)];
  uint v_6 = (24u + start_byte_offset);
  uint4 v_7 = m[(v_6 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)), asfloat(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)), asfloat(select((((v_6 & 15u) >> 2u) == 2u), v_7.zw, v_7.xy)));
}

[numthreads(1, 1, 1)]
void f() {
  float4x2 l_m = v(0u);
  float2 l_m_1 = asfloat(m[0u].zw);
}

