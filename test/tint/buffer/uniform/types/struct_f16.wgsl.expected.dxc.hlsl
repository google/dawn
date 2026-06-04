struct Inner {
  float16_t scalar_f16;
  vector<float16_t, 3> vec3_f16;
  matrix<float16_t, 2, 4> mat2x4_f16;
};

struct S {
  Inner inner;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
RWByteAddressBuffer s : register(u1);
void v_1(uint offset, matrix<float16_t, 2, 4> obj) {
  s.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

void v_2(uint offset, Inner obj) {
  s.Store<float16_t>((offset + 0u), obj.scalar_f16);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj.vec3_f16);
  v_1((offset + 16u), obj.mat2x4_f16);
}

void v_3(uint offset, S obj) {
  Inner v_4 = obj.inner;
  v_2((offset + 0u), v_4);
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 2, 4> v_5(uint start_byte_offset) {
  uint4 v_6 = u[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_7 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_6.zw, v_6.xy));
  uint v_8 = (8u + start_byte_offset);
  uint4 v_9 = u[(v_8 / 16u)];
  return matrix<float16_t, 2, 4>(v_7, tint_bitcast_to_f16_1(select((((v_8 & 15u) >> 2u) == 2u), v_9.zw, v_9.xy)));
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  uint2 v_10 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_10 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

Inner v_11(uint start_byte_offset) {
  float16_t v_12 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)])[select(((start_byte_offset % 4u) == 0u), 0u, 1u)];
  uint v_13 = (8u + start_byte_offset);
  uint4 v_14 = u[(v_13 / 16u)];
  vector<float16_t, 3> v_15 = tint_bitcast_to_f16_1(select((((v_13 & 15u) >> 2u) == 2u), v_14.zw, v_14.xy)).xyz;
  Inner v_16 = {v_12, v_15, v_5((16u + start_byte_offset))};
  return v_16;
}

S v_17(uint start_byte_offset) {
  Inner v_18 = v_11(start_byte_offset);
  S v_19 = {v_18};
  return v_19;
}

[numthreads(1, 1, 1)]
void main() {
  S x = v_17(0u);
  v_3(0u, x);
}

