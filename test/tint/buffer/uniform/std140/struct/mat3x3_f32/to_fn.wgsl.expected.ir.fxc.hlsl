struct S {
  int before;
  float3x3 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
void a(S a_1[4]) {
}

void b(S s) {
}

void c(float3x3 m) {
}

void d(float3 v) {
}

void e(float f) {
}

float3x3 v_1(uint start_byte_offset) {
  float3 v_2 = asfloat(u[(start_byte_offset / 16u)].xyz);
  float3 v_3 = asfloat(u[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_2, v_3, asfloat(u[((32u + start_byte_offset) / 16u)].xyz));
}

S v_4(uint start_byte_offset) {
  int v_5 = asint(u[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  float3x3 v_6 = v_1((16u + start_byte_offset));
  S v_7 = {v_5, v_6, asint(u[((64u + start_byte_offset) / 16u)][(((64u + start_byte_offset) % 16u) / 4u)])};
  return v_7;
}

typedef S ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      S v_11 = v_4((start_byte_offset + (v_10 * 128u)));
      a[v_10] = v_11;
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  S v_12[4] = a;
  return v_12;
}

[numthreads(1, 1, 1)]
void f() {
  S v_13[4] = v_8(0u);
  a(v_13);
  S v_14 = v_4(256u);
  b(v_14);
  c(v_1(272u));
  d(asfloat(u[2u].xyz).zxy);
  e(asfloat(u[2u].xyz).zxy[0u]);
}

