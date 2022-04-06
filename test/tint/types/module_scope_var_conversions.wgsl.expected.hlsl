static bool bool_var1 = bool(1u);
static bool bool_var2 = bool(1);
static bool bool_var3 = bool(1.0f);
static int i32_var1 = int(1u);
static int i32_var2 = int(1.0f);
static int i32_var3 = int(true);
static uint u32_var1 = uint(1);
static uint u32_var2 = uint(1.0f);
static uint u32_var3 = uint(true);
static bool3 v3bool_var1 = bool3(uint3((1u).xxx));
static bool3 v3bool_var2 = bool3(int3((1).xxx));
static bool3 v3bool_var3 = bool3(float3((1.0f).xxx));
static int3 v3i32_var1 = int3(uint3((1u).xxx));
static int3 v3i32_var2 = int3(float3((1.0f).xxx));
static int3 v3i32_var3 = int3(bool3((true).xxx));
static uint3 v3u32_var1 = uint3(int3((1).xxx));
static uint3 v3u32_var2 = uint3(float3((1.0f).xxx));
static uint3 v3u32_var3 = uint3(bool3((true).xxx));
static bool3 v3bool_var4 = bool3(bool2(float2((123.0f).xx)), true);
static bool4 v4bool_var5 = bool4(bool2(float2(123.0f, 0.0f)), bool2(true, bool(float(0.0f))));

[numthreads(1, 1, 1)]
void main() {
  bool_var1 = false;
  bool_var2 = false;
  bool_var3 = false;
  i32_var1 = 0;
  i32_var2 = 0;
  i32_var3 = 0;
  u32_var1 = 0u;
  u32_var2 = 0u;
  u32_var3 = 0u;
  v3bool_var1 = bool3(false, false, false);
  v3bool_var2 = bool3(false, false, false);
  v3bool_var3 = bool3(false, false, false);
  v3bool_var4 = bool3(false, false, false);
  v4bool_var5 = bool4(false, false, false, false);
  v3i32_var1 = int3(0, 0, 0);
  v3i32_var2 = int3(0, 0, 0);
  v3i32_var3 = int3(0, 0, 0);
  v3u32_var1 = uint3(0u, 0u, 0u);
  v3u32_var2 = uint3(0u, 0u, 0u);
  v3u32_var3 = uint3(0u, 0u, 0u);
  return;
}
