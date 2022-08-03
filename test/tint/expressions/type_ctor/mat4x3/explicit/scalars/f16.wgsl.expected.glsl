#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
f16mat4x3 m = f16mat4x3(f16vec3(0.0hf, 1.0hf, 2.0hf), f16vec3(3.0hf, 4.0hf, 5.0hf), f16vec3(6.0hf, 7.0hf, 8.0hf), f16vec3(9.0hf, 10.0hf, 11.0hf));
