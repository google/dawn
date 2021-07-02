struct S_inner {
  float a;
};
struct tint_array_wrapper {
  float arr[4];
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
  tint_array_wrapper member_arr;
  S_inner member_struct;
};

[numthreads(1, 1, 1)]
void main() {
  const S s = (S)0;
  return;
}
