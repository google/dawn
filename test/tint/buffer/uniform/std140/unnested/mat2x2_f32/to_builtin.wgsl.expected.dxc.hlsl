
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
float2x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  uint v_2 = (8u + start_byte_offset);
  uint4 v_3 = u[(v_2 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_1.zw, v_1.xy)), asfloat(select((((v_2 & 15u) >> 2u) == 2u), v_3.zw, v_3.xy)));
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 t = transpose(v(0u));
  float l = length(asfloat(u[0u].zw));
  float a = abs(asfloat(u[0u].xy).yx.x);
}

