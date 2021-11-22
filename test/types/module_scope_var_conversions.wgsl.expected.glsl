#version 310 es
precision mediump float;

bool bool_var1 = bool(1u);
bool bool_var2 = bool(1);
bool bool_var3 = bool(1.0f);
int i32_var1 = int(1u);
int i32_var2 = int(1.0f);
int i32_var3 = int(true);
uint u32_var1 = uint(1);
uint u32_var2 = uint(1.0f);
uint u32_var3 = uint(true);
bvec3 v3bool_var1 = bvec3(uvec3(1u));
bvec3 v3bool_var2 = bvec3(ivec3(1));
bvec3 v3bool_var3 = bvec3(vec3(1.0f));
ivec3 v3i32_var1 = ivec3(uvec3(1u));
ivec3 v3i32_var2 = ivec3(vec3(1.0f));
ivec3 v3i32_var3 = ivec3(bvec3(true));
uvec3 v3u32_var1 = uvec3(ivec3(1));
uvec3 v3u32_var2 = uvec3(vec3(1.0f));
uvec3 v3u32_var3 = uvec3(bvec3(true));
bvec3 v3bool_var4 = bvec3(bvec2(vec2(123.0f)), true);
bvec4 v4bool_var5 = bvec4(bvec2(vec2(123.0f, 0.0f)), bvec2(true, bool(float(0.0f))));

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  bool_var1 = false;
  bool_var2 = false;
  bool_var3 = false;
  i32_var1 = 0;
  i32_var2 = 0;
  i32_var3 = 0;
  u32_var1 = 0u;
  u32_var2 = 0u;
  u32_var3 = 0u;
  v3bool_var1 = bvec3(false, false, false);
  v3bool_var2 = bvec3(false, false, false);
  v3bool_var3 = bvec3(false, false, false);
  v3bool_var4 = bvec3(false, false, false);
  v4bool_var5 = bvec4(false, false, false, false);
  v3i32_var1 = ivec3(0, 0, 0);
  v3i32_var2 = ivec3(0, 0, 0);
  v3i32_var3 = ivec3(0, 0, 0);
  v3u32_var1 = uvec3(0u, 0u, 0u);
  v3u32_var2 = uvec3(0u, 0u, 0u);
  v3u32_var3 = uvec3(0u, 0u, 0u);
  return;
}
void main() {
  tint_symbol();
}


