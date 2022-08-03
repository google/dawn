#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
float16_t get_f16() {
  return 1.0hf;
}

void f() {
  float16_t tint_symbol = get_f16();
  f16vec2 v2 = f16vec2(tint_symbol);
  float16_t tint_symbol_1 = get_f16();
  f16vec3 v3 = f16vec3(tint_symbol_1);
  float16_t tint_symbol_2 = get_f16();
  f16vec4 v4 = f16vec4(tint_symbol_2);
}

