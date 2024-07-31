
cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
float2x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(u[(start_byte_offset / 16u)]);
  return float2x4(v_1, asfloat(u[((16u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void f() {
  float4x2 t = transpose(v(0u));
  float l = length(asfloat(u[1u]));
  float a = abs(asfloat(u[0u]).ywxz[0u]);
}

