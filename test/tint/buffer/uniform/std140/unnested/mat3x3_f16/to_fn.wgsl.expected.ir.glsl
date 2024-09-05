#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform tint_symbol_1_std140_1_ubo {
  f16vec3 tint_symbol_col0;
  f16vec3 tint_symbol_col1;
  f16vec3 tint_symbol_col2;
} v_1;
void a(f16mat3 m) {
}
void b(f16vec3 v) {
}
void c(float16_t f) {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(f16mat3(v_1.tint_symbol_col0, v_1.tint_symbol_col1, v_1.tint_symbol_col2));
  b(f16mat3(v_1.tint_symbol_col0, v_1.tint_symbol_col1, v_1.tint_symbol_col2)[1]);
  b(f16mat3(v_1.tint_symbol_col0, v_1.tint_symbol_col1, v_1.tint_symbol_col2)[1].zxy);
  c(f16mat3(v_1.tint_symbol_col0, v_1.tint_symbol_col1, v_1.tint_symbol_col2)[1][0u]);
  c(f16mat3(v_1.tint_symbol_col0, v_1.tint_symbol_col1, v_1.tint_symbol_col2)[1].zxy[0u]);
}
