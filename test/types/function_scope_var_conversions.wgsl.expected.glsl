#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  bool bool_var1 = bool(123u);
  bool bool_var2 = bool(123);
  bool bool_var3 = bool(123.0f);
  int i32_var1 = int(123u);
  int i32_var2 = int(123.0f);
  int i32_var3 = int(true);
  uint u32_var1 = uint(123);
  uint u32_var2 = uint(123.0f);
  uint u32_var3 = uint(true);
  bvec3 v3bool_var1 = bvec3(uvec3(123u));
  bvec3 v3bool_var11 = bvec3(uvec3(1234u));
  bvec3 v3bool_var2 = bvec3(ivec3(123));
  bvec3 v3bool_var3 = bvec3(vec3(123.0f));
  ivec3 v3i32_var1 = ivec3(uvec3(123u));
  ivec3 v3i32_var2 = ivec3(vec3(123.0f));
  ivec3 v3i32_var3 = ivec3(bvec3(true));
  uvec3 v3u32_var1 = uvec3(ivec3(123));
  uvec3 v3u32_var2 = uvec3(vec3(123.0f));
  uvec3 v3u32_var3 = uvec3(bvec3(true));
  bvec3 v3bool_var4 = bvec3(bvec2(vec2(123.0f)), true);
  bvec4 v4bool_var5 = bvec4(bvec2(vec2(123.0f, 0.0f)), bvec2(true, bool(float(0.0f))));
  return;
}
void main() {
  tint_symbol();
}


