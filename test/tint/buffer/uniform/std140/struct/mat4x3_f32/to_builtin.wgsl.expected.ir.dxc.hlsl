
cbuffer cbuffer_u : register(b0) {
  uint4 u[48];
};
float4x3 v(uint start_byte_offset) {
  float3 v_1 = asfloat(u[(start_byte_offset / 16u)].xyz);
  float3 v_2 = asfloat(u[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_3 = asfloat(u[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_1, v_2, v_3, asfloat(u[((48u + start_byte_offset) / 16u)].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  float3x4 t = transpose(v(400u));
  float l = length(asfloat(u[2u].xyz).zxy);
  float a = abs(asfloat(u[2u].xyz).zxy[0u]);
}

