
cbuffer cbuffer_data : register(b0) {
  uint4 data[3];
};
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 3> v_1(uint start_byte_offset) {
  uint4 v_2 = data[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_3 = tint_bitcast_to_f16(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)).xyz;
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = data[(v_4 / 16u)];
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)).xyz;
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = data[(v_7 / 16u)];
  vector<float16_t, 3> v_9 = tint_bitcast_to_f16(select((((v_7 & 15u) >> 2u) == 2u), v_8.zw, v_8.xy)).xyz;
  uint v_10 = (24u + start_byte_offset);
  uint4 v_11 = data[(v_10 / 16u)];
  return matrix<float16_t, 4, 3>(v_3, v_6, v_9, tint_bitcast_to_f16(select((((v_10 & 15u) >> 2u) == 2u), v_11.zw, v_11.xy)).xyz);
}

void main() {
  vector<float16_t, 3> v_12 = tint_bitcast_to_f16(data[2u].xy).xyz;
  vector<float16_t, 4> x = mul(v_1(0u), v_12);
}

