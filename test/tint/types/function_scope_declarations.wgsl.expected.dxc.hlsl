struct S {
  float a;
};

[numthreads(1, 1, 1)]
void main() {
  bool bool_var = false;
  const bool bool_let = false;
  int i32_var = 0;
  const int i32_let = 0;
  uint u32_var = 0u;
  const uint u32_let = 0u;
  float f32_var = 0.0f;
  const float f32_let = 0.0f;
  int2 v2i32_var = (0).xx;
  const int2 v2i32_let = (0).xx;
  uint3 v3u32_var = (0u).xxx;
  const uint3 v3u32_let = (0u).xxx;
  float4 v4f32_var = (0.0f).xxxx;
  const float4 v4f32_let = (0.0f).xxxx;
  float2x3 m2x3_var = float2x3((0.0f).xxx, (0.0f).xxx);
  const float3x4 m3x4_let = float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float arr_var[4] = (float[4])0;
  const float arr_let[4] = (float[4])0;
  S struct_var = (S)0;
  const S struct_let = (S)0;
  return;
}
