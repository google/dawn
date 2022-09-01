#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
float16_t t = 0.0hf;
f16vec4 m() {
  t = 1.0hf;
  return f16vec4(t);
}

void f() {
  f16vec4 tint_symbol = m();
  uvec4 v = uvec4(tint_symbol);
}

