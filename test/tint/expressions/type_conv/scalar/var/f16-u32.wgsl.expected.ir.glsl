#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

float16_t u = 1.0hf;
uint tint_f16_to_u32(float16_t value) {
  return (((value <= 65504.0hf)) ? ((((value >= 0.0hf)) ? (uint(value)) : (0u))) : (4294967295u));
}
void f() {
  uint v = tint_f16_to_u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
