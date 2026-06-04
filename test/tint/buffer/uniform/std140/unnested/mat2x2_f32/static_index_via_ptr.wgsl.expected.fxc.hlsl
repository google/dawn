
cbuffer cbuffer_m : register(b0) {
  uint4 m[1];
};
float2x2 v(uint start_byte_offset) {
  uint4 v_1 = m[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = m[(v_3 / 16u)];
  return float2x2(v_2, asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy))));
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 l_m = v(0u);
  float2 l_m_1 = asfloat(m[0u].zw);
}

