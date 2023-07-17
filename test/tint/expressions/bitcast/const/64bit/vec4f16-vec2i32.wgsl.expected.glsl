#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void f() {
  ivec2 b = ivec2(1073757184, -1006616064);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
