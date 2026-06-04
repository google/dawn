
cbuffer cbuffer_data : register(b0) {
  uint4 data[3];
};
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = data[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = data[(v_2 / 16u)];
  uint v_4 = (16u + start_byte_offset);
  uint4 v_5 = data[(v_4 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)), asfloat(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)));
}

void main() {
  float3x2 v_6 = v(0u);
  float2 x = mul(asfloat(data[2u].xyz), v_6);
}

