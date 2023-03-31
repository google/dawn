cbuffer cbuffer_u : register(b0) {
  uint4 u[16];
};

float4x3 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(u[scalar_offset / 4].xyz), asfloat(u[scalar_offset_1 / 4].xyz), asfloat(u[scalar_offset_2 / 4].xyz), asfloat(u[scalar_offset_3 / 4].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  const float3x4 t = transpose(u_load(128u));
  const float l = length(asfloat(u[1].xyz).zxy);
  const float a = abs(asfloat(u[1].xyz).zxy.x);
  return;
}
