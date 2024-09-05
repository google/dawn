#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16mat4 m = f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf));
layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  f16mat4 tint_symbol_1;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol_1 = m;
}
