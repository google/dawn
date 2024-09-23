
cbuffer cbuffer_u : register(b0) {
  uint4 u[12];
};
RWByteAddressBuffer s : register(u1);
float3x4 v(uint start_byte_offset) {
  float4 v_1 = asfloat(u[(start_byte_offset / 16u)]);
  float4 v_2 = asfloat(u[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_1, v_2, asfloat(u[((32u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void f() {
  float4x3 t = transpose(v(96u));
  float l = length(asfloat(u[1u]).ywxz);
  float a = abs(asfloat(u[1u]).ywxz[0u]);
  float v_3 = (t[int(0)][0u] + float(l));
  s.Store(0u, asuint((v_3 + float(a))));
}

