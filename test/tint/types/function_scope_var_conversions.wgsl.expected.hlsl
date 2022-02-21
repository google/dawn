void constant_with_non_constant() {
  float a = 0.0f;
  float2 b = float2(float(int(1)), a);
}

[numthreads(1, 1, 1)]
void main() {
  bool bool_var1 = bool(123u);
  bool bool_var2 = bool(123);
  bool bool_var3 = bool(123.0f);
  int i32_var1 = int(123u);
  int i32_var2 = int(123.0f);
  int i32_var3 = int(true);
  uint u32_var1 = uint(123);
  uint u32_var2 = uint(123.0f);
  uint u32_var3 = uint(true);
  bool3 v3bool_var1 = bool3(uint3((123u).xxx));
  bool3 v3bool_var11 = bool3(uint3((1234u).xxx));
  bool3 v3bool_var2 = bool3(int3((123).xxx));
  bool3 v3bool_var3 = bool3(float3((123.0f).xxx));
  int3 v3i32_var1 = int3(uint3((123u).xxx));
  int3 v3i32_var2 = int3(float3((123.0f).xxx));
  int3 v3i32_var3 = int3(bool3((true).xxx));
  uint3 v3u32_var1 = uint3(int3((123).xxx));
  uint3 v3u32_var2 = uint3(float3((123.0f).xxx));
  uint3 v3u32_var3 = uint3(bool3((true).xxx));
  bool3 v3bool_var4 = bool3(bool2(float2((123.0f).xx)), true);
  bool4 v4bool_var5 = bool4(bool2(float2(123.0f, 0.0f)), bool2(true, bool(float(0.0f))));
  return;
}
