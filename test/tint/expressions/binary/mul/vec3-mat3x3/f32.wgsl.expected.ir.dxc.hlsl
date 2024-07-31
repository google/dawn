
cbuffer cbuffer_data : register(b0) {
  uint4 data[4];
};
float3x3 v(uint start_byte_offset) {
  float3 v_1 = asfloat(data[(start_byte_offset / 16u)].xyz);
  float3 v_2 = asfloat(data[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_1, v_2, asfloat(data[((32u + start_byte_offset) / 16u)].xyz));
}

void main() {
  float3 v_3 = asfloat(data[3u].xyz);
  float3 x = mul(v(0u), v_3);
}

