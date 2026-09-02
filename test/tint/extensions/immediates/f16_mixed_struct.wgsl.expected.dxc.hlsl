
cbuffer cbuffer_data : register(b0, space1) {
  uint4 data[1];
};
RWByteAddressBuffer output : register(u0);
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

[numthreads(1, 1, 1)]
void main() {
  float v_1 = asfloat(data[0u].x);
  float2 v_2 = float2(tint_bitcast_to_f16(data[0u].y));
  output.Store4(0u, asuint(float4(v_1, v_2, float(tint_bitcast_to_f16(data[0u].z).x))));
}

