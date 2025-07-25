#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec2 u = f16vec2(1.0hf);
ivec2 tint_v2f16_to_v2i32(f16vec2 value) {
  return ivec2(clamp(value, f16vec2(-65504.0hf), f16vec2(65504.0hf)));
}
void f() {
  ivec2 v = tint_v2f16_to_v2i32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
