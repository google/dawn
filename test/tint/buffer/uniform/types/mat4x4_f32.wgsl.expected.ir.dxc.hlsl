
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float4x4 obj) {
  s.Store4((offset + 0u), asuint(obj[0u]));
  s.Store4((offset + 16u), asuint(obj[1u]));
  s.Store4((offset + 32u), asuint(obj[2u]));
  s.Store4((offset + 48u), asuint(obj[3u]));
}

float4x4 v_1(uint start_byte_offset) {
  return float4x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]), asfloat(u[((32u + start_byte_offset) / 16u)]), asfloat(u[((48u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void main() {
  float4x4 x = v_1(0u);
  v(0u, x);
}

