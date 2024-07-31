struct Inner {
  float scalar_f32;
  float3 vec3_f32;
  float2x4 mat2x4_f32;
};

struct S {
  Inner inner;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float2x4 obj) {
  s.Store4((offset + 0u), asuint(obj[0u]));
  s.Store4((offset + 16u), asuint(obj[1u]));
}

void v_1(uint offset, Inner obj) {
  s.Store((offset + 0u), asuint(obj.scalar_f32));
  s.Store3((offset + 16u), asuint(obj.vec3_f32));
  v((offset + 32u), obj.mat2x4_f32);
}

void v_2(uint offset, S obj) {
  Inner v_3 = obj.inner;
  v_1((offset + 0u), v_3);
}

float2x4 v_4(uint start_byte_offset) {
  float4 v_5 = asfloat(u[(start_byte_offset / 16u)]);
  return float2x4(v_5, asfloat(u[((16u + start_byte_offset) / 16u)]));
}

Inner v_6(uint start_byte_offset) {
  float v_7 = asfloat(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float3 v_8 = asfloat(u[((16u + start_byte_offset) / 16u)].xyz);
  Inner v_9 = {v_7, v_8, v_4((32u + start_byte_offset))};
  return v_9;
}

S v_10(uint start_byte_offset) {
  Inner v_11 = v_6(start_byte_offset);
  S v_12 = {v_11};
  return v_12;
}

[numthreads(1, 1, 1)]
void main() {
  S v_13 = v_10(0u);
  S x = v_13;
  v_2(0u, x);
}

