#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer tint_symbol_4_1_ssbo {
  f16mat3 tint_symbol_3;
} v;
layout(binding = 1, std430)
buffer tint_symbol_6_1_ssbo {
  f16mat3 tint_symbol_5;
} v_1;
void tint_store_and_preserve_padding(inout f16mat3 target, f16mat3 value_param) {
  target[0u] = value_param[0u];
  target[1u] = value_param[1u];
  target[2u] = value_param[2u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v_1.tint_symbol_5, v.tint_symbol_3);
}
