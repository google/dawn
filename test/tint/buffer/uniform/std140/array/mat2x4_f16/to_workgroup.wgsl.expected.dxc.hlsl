struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
groupshared matrix<float16_t, 2, 4> w[4];
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 4> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_4 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_3.zw, v_3.xy));
  uint v_5 = (8u + start_byte_offset);
  uint4 v_6 = u[(v_5 / 16u)];
  return matrix<float16_t, 2, 4>(v_4, tint_bitcast_to_f16_1(select((((v_5 & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)));
}

typedef matrix<float16_t, 2, 4> ary_ret[4];
ary_ret v_7(uint start_byte_offset) {
  matrix<float16_t, 2, 4> a[4] = (matrix<float16_t, 2, 4>[4])0;
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_2((start_byte_offset + (v_9 * 16u)));
      {
        v_8 = (v_9 + 1u);
      }
    }
  }
  matrix<float16_t, 2, 4> v_10[4] = a;
  return v_10;
}

void f_inner(uint tint_local_index) {
  {
    uint v_11 = 0u;
    v_11 = tint_local_index;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      w[v_12] = matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
      {
        v_11 = (v_12 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  matrix<float16_t, 2, 4> v_13[4] = v_7(0u);
  w = v_13;
  w[1u] = v_2(32u);
  w[1u][0u] = tint_bitcast_to_f16_1(u[0u].zw).ywxz;
  w[1u][0u].x = tint_bitcast_to_f16(u[0u].z).x;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

