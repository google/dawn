
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
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

void v_5(uint offset, matrix<float16_t, 4, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  s.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
  s.Store<vector<float16_t, 3> >((offset + 24u), obj[3u]);
}

matrix<float16_t, 4, 3> v_6(uint start_byte_offset) {
  uint4 v_7 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_8 = tint_bitcast_to_f16_1((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy))).xyz;
  uint4 v_9 = u[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_10 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_9.zw) : (v_9.xy))).xyz;
  uint4 v_11 = u[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_12 = tint_bitcast_to_f16_1(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_11.zw) : (v_11.xy))).xyz;
  uint4 v_13 = u[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 3>(v_8, v_10, v_12, tint_bitcast_to_f16_1(((((((24u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_13.zw) : (v_13.xy))).xyz);
}

void v_14(uint offset, matrix<float16_t, 4, 3> obj[4]) {
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      v_5((offset + (v_16 * 32u)), obj[v_16]);
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
}

typedef matrix<float16_t, 4, 3> ary_ret[4];
ary_ret v_17(uint start_byte_offset) {
  matrix<float16_t, 4, 3> a[4] = (matrix<float16_t, 4, 3>[4])0;
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 4u)) {
        break;
      }
      a[v_19] = v_6((start_byte_offset + (v_19 * 32u)));
      {
        v_18 = (v_19 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 3> v_20[4] = a;
  return v_20;
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 4, 3> v_21[4] = v_17(0u);
  v_14(0u, v_21);
  v_5(32u, v_6(64u));
  s.Store<vector<float16_t, 3> >(32u, tint_bitcast_to_f16_1(u[0u].zw).xyz.zxy);
  s.Store<float16_t>(32u, tint_bitcast_to_f16(u[0u].z).x);
}

