#version 310 es

shared float wg_var;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    wg_var = 0.0f;
  }
  barrier();
}

struct S {
  float a;
};

bool bool_var = false;
int i32_var = 0;
uint u32_var = 0u;
float f32_var = 0.0f;
ivec2 v2i32_var = ivec2(0, 0);
uvec3 v3u32_var = uvec3(0u, 0u, 0u);
vec4 v4f32_var = vec4(0.0f, 0.0f, 0.0f, 0.0f);
mat2x3 m2x3_var = mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
float arr_var[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
S struct_var = S(0.0f);
void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  bool_var = false;
  i32_var = 0;
  u32_var = 0u;
  f32_var = 0.0f;
  v2i32_var = ivec2(0);
  v3u32_var = uvec3(0u);
  v4f32_var = vec4(0.0f);
  m2x3_var = mat2x3(vec3(0.0f), vec3(0.0f));
  float tint_symbol_1[4] = float[4](0.0f, 0.0f, 0.0f, 0.0f);
  arr_var = tint_symbol_1;
  S tint_symbol_2 = S(0.0f);
  struct_var = tint_symbol_2;
  wg_var = 42.0f;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
