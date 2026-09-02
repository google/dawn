
cbuffer cbuffer_data : register(b0) {
  uint4 data[2];
};
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

vector<float16_t, 2> tint_bitcast_to_f16_1(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 3, 2> v_1(uint start_byte_offset) {
  vector<float16_t, 2> v_2 = tint_bitcast_to_f16_1(data[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16_1(data[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  return matrix<float16_t, 3, 2>(v_2, v_4, tint_bitcast_to_f16_1(data[(v_5 / 16u)][((v_5 & 15u) >> 2u)]));
}

void main() {
  matrix<float16_t, 3, 2> v_6 = v_1(0u);
  vector<float16_t, 2> x = mul(tint_bitcast_to_f16(data[1u].xy).xyz, v_6);
}

