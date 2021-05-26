struct S {
};

static const bool bool_let = false;
static const int i32_let = 0;
static const uint u32_let = 0u;
static const float f32_let = 0.0f;
static const int2 v2i32_let = int2(0, 0);
static const uint3 v3u32_let = uint3(0u, 0u, 0u);
static const float4 v4f32_let = float4(0.0f, 0.0f, 0.0f, 0.0f);
static const float3x4 m3x4_let = float3x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
static const float arr_let[4] = {0.0f, 0.0f, 0.0f, 0.0f};
static const S struct_let = {};
[numthreads(1, 1, 1)]
void main() {
  return;
}

