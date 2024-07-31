
cbuffer cbuffer_data : register(b0) {
  uint4 data[5];
};
float4x3 v(uint start_byte_offset) {
  float3 v_1 = asfloat(data[(start_byte_offset / 16u)].xyz);
  float3 v_2 = asfloat(data[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_3 = asfloat(data[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_1, v_2, v_3, asfloat(data[((48u + start_byte_offset) / 16u)].xyz));
}

void main() {
  float3 v_4 = asfloat(data[4u].xyz);
  float4 x = mul(v(0u), v_4);
}

