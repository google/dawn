struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
groupshared matrix<float16_t, 2, 2> w;
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_3, tint_bitcast_to_f16(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)]));
}

void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
  }
  GroupMemoryBarrierWithGroupSync();
  w = v_2(0u);
  w[1u] = tint_bitcast_to_f16(u[0u].x);
  w[1u] = tint_bitcast_to_f16(u[0u].x).yx;
  w[0u].y = tint_bitcast_to_f16(u[0u].y).x;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

