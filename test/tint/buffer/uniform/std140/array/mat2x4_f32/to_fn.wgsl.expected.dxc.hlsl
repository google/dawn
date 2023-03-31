cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};

void a(float2x4 a_1[4]) {
}

void b(float2x4 m) {
}

void c(float4 v) {
}

void d(float f_1) {
}

float2x4 u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]));
}

typedef float2x4 u_load_ret[4];
u_load_ret u_load(uint offset) {
  float2x4 arr[4] = (float2x4[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = u_load_1((offset + (i * 32u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  a(u_load(0u));
  b(u_load_1(32u));
  c(asfloat(u[2]).ywxz);
  d(asfloat(u[2]).ywxz.x);
  return;
}
