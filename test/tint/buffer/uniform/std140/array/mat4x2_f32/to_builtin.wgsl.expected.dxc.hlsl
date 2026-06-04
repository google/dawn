
cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);
float4x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = u[(v_2 / 16u)];
  uint v_4 = (16u + start_byte_offset);
  uint4 v_5 = u[(v_4 / 16u)];
  uint v_6 = (24u + start_byte_offset);
  uint4 v_7 = u[(v_6 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)), asfloat(select((((v_4 & 15u) >> 2u) == 2u), v_5.zw, v_5.xy)), asfloat(select((((v_6 & 15u) >> 2u) == 2u), v_7.zw, v_7.xy)));
}

[numthreads(1, 1, 1)]
void f() {
  float2x4 t = transpose(v(64u));
  float l = length(asfloat(u[0u].zw).yx);
  float a = abs(asfloat(u[0u].zw).yx.x);
  float v_8 = (t[0u].x + float(l));
  s.Store(0u, asuint((v_8 + float(a))));
}

