#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

f16vec3 u = f16vec3(1.0hf);
uvec3 tint_v3f16_to_v3u32(f16vec3 value) {
  return uvec3(clamp(value, f16vec3(0.0hf), f16vec3(65504.0hf)));
}
void f() {
  uvec3 v = tint_v3f16_to_v3u32(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
