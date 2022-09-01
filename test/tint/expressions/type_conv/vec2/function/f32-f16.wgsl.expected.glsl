#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
float t = 0.0f;
vec2 m() {
  t = 1.0f;
  return vec2(t);
}

void f() {
  vec2 tint_symbol = m();
  f16vec2 v = f16vec2(tint_symbol);
}

