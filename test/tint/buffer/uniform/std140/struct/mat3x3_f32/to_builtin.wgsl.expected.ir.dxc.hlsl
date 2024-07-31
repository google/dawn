
cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
float3x3 v(uint start_byte_offset) {
  float3 v_1 = asfloat(u[(start_byte_offset / 16u)].xyz);
  float3 v_2 = asfloat(u[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_1, v_2, asfloat(u[((32u + start_byte_offset) / 16u)].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  float3x3 t = transpose(v(272u));
  float l = length(asfloat(u[2u].xyz).zxy);
  float a = abs(asfloat(u[2u].xyz).zxy[0u]);
}

