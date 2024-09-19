
cbuffer cbuffer_m : register(b0) {
  uint4 m[3];
};
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

float3x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(m[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(m[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_1, v_2, asfloat(m[((32u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void f() {
  float3x4 l_m = v(0u);
  float4 l_m_1 = asfloat(m[1u]);
}

