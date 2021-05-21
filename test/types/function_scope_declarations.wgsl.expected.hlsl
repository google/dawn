struct S {
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
  int2 v2i32_var = int2(0, 0);
  const int2 v2i32_let = int2(0, 0);
  uint3 v3u32_var = uint3(0u, 0u, 0u);
  const uint3 v3u32_let = uint3(0u, 0u, 0u);
  float4 v4f32_var = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 v4f32_let = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2x3 m2x3_var = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  const float3x4 m3x4_let = float3x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float arr_var[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  const float arr_let[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  S struct_var = {};
  const S struct_let = {};
  return;
}

