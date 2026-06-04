
cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
float2x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = u[(v_3 / 16u)];
  return float2x2(v_2, asfloat((((((v_3 & 15u) >> 2u) == 2u)) ? (v_4.zw) : (v_4.xy))));
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 t = transpose(v(264u));
  float l = length(asfloat(u[1u].xy).yx);
  float a = abs(asfloat(u[1u].xy).yx.x);
}

