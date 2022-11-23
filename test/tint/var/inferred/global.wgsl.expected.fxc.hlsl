struct MyStruct {
  float f1;
};

static int v1 = 1;
static uint v2 = 1u;
static float v3 = 1.0f;
static int3 v4 = (1).xxx;
static uint3 v5 = uint3(1u, 2u, 3u);
static float3 v6 = float3(1.0f, 2.0f, 3.0f);
const MyStruct c = {1.0f};
static MyStruct v7 = c;
static float v8[10] = (float[10])0;
static int v9 = 0;
static uint v10 = 0u;
static float v11 = 0.0f;
static MyStruct v12 = (MyStruct)0;
static MyStruct v13 = (MyStruct)0;
static float v14[10] = (float[10])0;
static int3 v15 = int3(1, 2, 3);
static float3 v16 = float3(1.0f, 2.0f, 3.0f);

[numthreads(1, 1, 1)]
void f() {
  const int l1 = v1;
  const uint l2 = v2;
  const float l3 = v3;
  const int3 l4 = v4;
  const uint3 l5 = v5;
  const float3 l6 = v6;
  const MyStruct l7 = v7;
  const float l8[10] = v8;
  const int l9 = v9;
  const uint l10 = v10;
  const float l11 = v11;
  const MyStruct l12 = v12;
  const MyStruct l13 = v13;
  const float l14[10] = v14;
  const int3 l15 = v15;
  const float3 l16 = v16;
  return;
}
