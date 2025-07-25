#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec2 u = f16vec2(1.0hf);
uvec2 tint_v2f16_to_v2u32(f16vec2 value) {
  return uvec2(clamp(value, f16vec2(0.0hf), f16vec2(65504.0hf)));
}
void f() {
  uvec2 v = tint_v2f16_to_v2u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
