
cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
static float3x2 p = float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx);
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = u[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = u[((16u + start_byte_offset) / 16u)];
  return float3x2(v_2, v_4, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))));
}

[numthreads(1, 1, 1)]
void f() {
  p = v(0u);
  p[1u] = asfloat(u[0u].xy);
  p[1u] = asfloat(u[0u].xy).yx;
  p[0u].y = asfloat(u[0u].z);
}

