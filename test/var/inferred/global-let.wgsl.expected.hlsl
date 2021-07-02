struct MyStruct {
  float f1;
};
struct tint_array_wrapper {
  float arr[10];
};

static const int v1 = 1;
static const uint v2 = 1u;
static const float v3 = 1.0f;
static const int3 v4 = int3(1, 1, 1);
static const uint3 v5 = uint3(1u, 1u, 1u);
static const float3 v6 = float3(1.0f, 1.0f, 1.0f);
static const float3x3 v7 = float3x3(float3(1.0f, 1.0f, 1.0f), float3(1.0f, 1.0f, 1.0f), float3(1.0f, 1.0f, 1.0f));
static const MyStruct v8 = (MyStruct)0;
static const tint_array_wrapper v9 = {(float[10])0};

struct tint_symbol {
  float4 value : SV_Target0;
};

tint_symbol main() {
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}
