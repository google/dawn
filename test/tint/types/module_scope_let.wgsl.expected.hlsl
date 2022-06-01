struct S {
  float a;
};

static const bool bool_let = false;
static const int i32_let = 0;
static const uint u32_let = 0u;
static const float f32_let = 0.0f;
static const int2 v2i32_let = (0).xx;
static const uint3 v3u32_let = (0u).xxx;
static const float4 v4f32_let = (0.0f).xxxx;
static const float3x4 m3x4_let = float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
static const float arr_let[4] = (float[4])0;
static const S struct_let = (S)0;

[numthreads(1, 1, 1)]
void main() {
  return;
}
