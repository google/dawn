#version 310 es

const uint width = 128u;
layout(binding = 1, std430) buffer Result_1 {
  float values[];
} result;
uniform highp sampler2D tex_1;
void tint_symbol(uvec3 GlobalInvocationId) {
  result.values[((GlobalInvocationId.y * width) + GlobalInvocationId.x)] = texelFetch(tex_1, ivec2(int(GlobalInvocationId.x), int(GlobalInvocationId.y)), 0).x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_GlobalInvocationID);
  return;
}
