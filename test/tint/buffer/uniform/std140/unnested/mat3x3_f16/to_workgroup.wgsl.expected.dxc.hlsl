struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
groupshared matrix<float16_t, 3, 3> w;
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 3, 3> v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_3 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)).xyz;
  uint v_4 = (8u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16_1(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)).xyz;
  uint v_7 = (16u + start_byte_offset);
  uint4 v_8 = u[(v_7 / 16u)];
  return matrix<float16_t, 3, 3>(v_3, v_6, tint_bitcast_to_f16_1(select((((v_7 & 15u) >> 2u) == 2u), v_8.zw, v_8.xy)).xyz);
}

void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
  }
  GroupMemoryBarrierWithGroupSync();
  w = v_1(0u);
  w[int(1)] = tint_bitcast_to_f16_1(u[0u].xy).xyz;
  w[int(1)] = tint_bitcast_to_f16_1(u[0u].xy).xyz.zxy;
  w[int(0)].y = tint_bitcast_to_f16(u[0u].z).x;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

