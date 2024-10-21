#version 310 es

layout(binding = 1, std430)
buffer Result_1_ssbo {
  float values[];
} result;
uniform highp sampler2D tex;
void tint_symbol_inner(uvec3 GlobalInvocationId) {
  int v = int(GlobalInvocationId[0u]);
  ivec2 v_1 = ivec2(ivec2(v, int(GlobalInvocationId[1u])));
  result.values[((GlobalInvocationId[1u] * 128u) + GlobalInvocationId[0u])] = texelFetch(tex, v_1, int(0)).x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_GlobalInvocationID);
}
