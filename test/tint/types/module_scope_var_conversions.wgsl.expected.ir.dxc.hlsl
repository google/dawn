SKIP: FAILED

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
  v3bool_var1 = (false).xxx;
  v3bool_var2 = (false).xxx;
  v3bool_var3 = (false).xxx;
  v3bool_var4 = (false).xxx;
  v4bool_var5 = (false).xxxx;
  v3i32_var1 = (0).xxx;
  v3i32_var2 = (0).xxx;
  v3i32_var3 = (0).xxx;
  v3u32_var1 = (0u).xxx;
  v3u32_var2 = (0u).xxx;
  v3u32_var3 = (0u).xxx;
}

DXC validation failure:
hlsl.hlsl:3:3: error: use of undeclared identifier 'bool_var1'
  bool_var1 = false;
  ^
hlsl.hlsl:4:3: error: use of undeclared identifier 'bool_var2'
  bool_var2 = false;
  ^
hlsl.hlsl:5:3: error: use of undeclared identifier 'bool_var3'
  bool_var3 = false;
  ^
hlsl.hlsl:6:3: error: use of undeclared identifier 'i32_var1'
  i32_var1 = 0;
  ^
hlsl.hlsl:7:3: error: use of undeclared identifier 'i32_var2'
  i32_var2 = 0;
  ^
hlsl.hlsl:8:3: error: use of undeclared identifier 'i32_var3'
  i32_var3 = 0;
  ^
hlsl.hlsl:9:3: error: use of undeclared identifier 'u32_var1'
  u32_var1 = 0u;
  ^
hlsl.hlsl:10:3: error: use of undeclared identifier 'u32_var2'
  u32_var2 = 0u;
  ^
hlsl.hlsl:11:3: error: use of undeclared identifier 'u32_var3'
  u32_var3 = 0u;
  ^
hlsl.hlsl:12:3: error: use of undeclared identifier 'v3bool_var1'
  v3bool_var1 = (false).xxx;
  ^
hlsl.hlsl:13:3: error: use of undeclared identifier 'v3bool_var2'
  v3bool_var2 = (false).xxx;
  ^
hlsl.hlsl:14:3: error: use of undeclared identifier 'v3bool_var3'
  v3bool_var3 = (false).xxx;
  ^
hlsl.hlsl:15:3: error: use of undeclared identifier 'v3bool_var4'
  v3bool_var4 = (false).xxx;
  ^
hlsl.hlsl:16:3: error: use of undeclared identifier 'v4bool_var5'
  v4bool_var5 = (false).xxxx;
  ^
hlsl.hlsl:17:3: error: use of undeclared identifier 'v3i32_var1'
  v3i32_var1 = (0).xxx;
  ^
hlsl.hlsl:18:3: error: use of undeclared identifier 'v3i32_var2'
  v3i32_var2 = (0).xxx;
  ^
hlsl.hlsl:19:3: error: use of undeclared identifier 'v3i32_var3'
  v3i32_var3 = (0).xxx;
  ^
hlsl.hlsl:20:3: error: use of undeclared identifier 'v3u32_var1'
  v3u32_var1 = (0u).xxx;
  ^
hlsl.hlsl:21:3: error: use of undeclared identifier 'v3u32_var2'
  v3u32_var2 = (0u).xxx;
  ^
hlsl.hlsl:22:3: error: use of undeclared identifier 'v3u32_var3'
  v3u32_var3 = (0u).xxx;
  ^

