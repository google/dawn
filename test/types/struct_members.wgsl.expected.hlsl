struct S_inner {
};
struct S {
  bool member_bool;
  int member_i32;
  uint member_u32;
  float member_f32;
  int2 member_v2i32;
  uint3 member_v3u32;
  float4 member_v4f32;
  float2x3 member_m2x3;
  float member_arr[4];
  S_inner member_struct;
};

[numthreads(1, 1, 1)]
void main() {
  const S s = {false, 0, 0u, 0.0f, int2(0, 0), uint3(0u, 0u, 0u), float4(0.0f, 0.0f, 0.0f, 0.0f), float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), {0.0f, 0.0f, 0.0f, 0.0f}, {}};
  return;
}

