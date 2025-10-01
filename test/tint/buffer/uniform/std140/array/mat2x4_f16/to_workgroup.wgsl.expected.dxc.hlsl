struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
groupshared matrix<float16_t, 2, 4> w[4];
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_2 = float16_t(t_low.x);
  float16_t v_3 = float16_t(t_high.x);
  float16_t v_4 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_2, v_3, v_4, float16_t(t_high.y));
}

matrix<float16_t, 2, 4> v_5(uint start_byte_offset) {
  uint4 v_6 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_7 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy)));
  uint4 v_8 = u[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 4>(v_7, tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_8.zw) : (v_8.xy))));
}

typedef matrix<float16_t, 2, 4> ary_ret[4];
ary_ret v_9(uint start_byte_offset) {
  matrix<float16_t, 2, 4> a[4] = (matrix<float16_t, 2, 4>[4])0;
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      a[v_11] = v_5((start_byte_offset + (v_11 * 16u)));
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 2, 4> v_12[4] = a;
  return v_12;
}

void f_inner(uint tint_local_index) {
  {
    uint v_13 = 0u;
    v_13 = tint_local_index;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      w[v_14] = matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
      {
        v_13 = (v_14 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  matrix<float16_t, 2, 4> v_15[4] = v_9(0u);
  w = v_15;
  w[1u] = v_5(32u);
  w[1u][0u] = tint_bitcast_to_f16_1(u[0u].zw).ywxz;
  w[1u][0u].x = tint_bitcast_to_f16(u[0u].z).x;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

