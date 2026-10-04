
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
float a(float2x2 a_1[4]) {
  return a_1[int(0)][int(0)].x;
}

float b(float2x2 m) {
  return m[int(0)].x;
}

float c(float2 v) {
  return v.x;
}

float d(float f_1) {
  return f_1;
}

float2x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  uint v_3 = (8u + start_byte_offset);
  uint4 v_4 = u[(v_3 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_2.zw, v_2.xy)), asfloat(select((((v_3 & 15u) >> 2u) == 2u), v_4.zw, v_4.xy)));
}

typedef float2x2 ary_ret[4];
ary_ret v_5(uint start_byte_offset) {
  float2x2 a_2[4] = (float2x2[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_6 = idx;
      if ((v_6 >= 4u)) {
        break;
      }
      a_2[v_6] = v_1((start_byte_offset + (v_6 * 16u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float2x2 v_7[4] = a_2;
  return v_7;
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 v_8[4] = v_5(0u);
  float v_9 = a(v_8);
  float v_10 = (v_9 + b(v_1(16u)));
  float v_11 = (v_10 + c(asfloat(u[1u].xy).yx));
  s.Store(0u, asuint((v_11 + d(asfloat(u[1u].y)))));
}

