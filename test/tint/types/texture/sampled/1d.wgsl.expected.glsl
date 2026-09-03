#version 310 es

layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v;
uniform highp sampler2D t_f;
uniform highp isampler2D t_i;
uniform highp usampler2D t_u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint fdims = uvec2(textureSize(t_f, int(min(1u, (v.metadata[0u].x - 1u))))).x;
  uint idims = uvec2(textureSize(t_i, int(min(1u, (v.metadata[0u].y - 1u))))).x;
  uint udims = uvec2(textureSize(t_u, int(min(1u, (v.metadata[0u].z - 1u))))).x;
}
