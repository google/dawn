
cbuffer cbuffer_value : register(b0, space1) {
  uint4 value[1];
};
RWByteAddressBuffer output : register(u0);
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

[numthreads(1, 1, 1)]
void main() {
  output.Store3(0u, asuint(float3(tint_bitcast_to_f16(value[0u].xy).xyz)));
}

