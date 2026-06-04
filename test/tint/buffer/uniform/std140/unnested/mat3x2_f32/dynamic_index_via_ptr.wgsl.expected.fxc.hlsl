
cbuffer cbuffer_m : register(b0) {
  uint4 m[2];
};
static int counter = int(0);
int i() {
  counter = asint((asuint(counter) + asuint(int(1))));
  return counter;
}

float3x2 v(uint start_byte_offset) {
  uint4 v_1 = m[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = m[(v_3 / 16u)];
  float2 v_5 = asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint v_6 = (16u + start_byte_offset);
  uint4 v_7 = m[(v_6 / 16u)];
  return float3x2(v_2, v_5, asfloat((((((v_6 & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

[numthreads(1, 1, 1)]
void f() {
  uint v_8 = (min(uint(i()), 2u) * 8u);
  float3x2 l_m = v(0u);
  uint4 v_9 = m[(v_8 / 16u)];
  float2 l_m_i = asfloat((((((v_8 & 15u) >> 2u) == 2u)) ? (v_9.zw) : (v_9.xy)));
}

