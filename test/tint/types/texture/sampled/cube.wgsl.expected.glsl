#version 310 es

layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v;
uniform highp samplerCube t_f;
uniform highp isamplerCube t_i;
uniform highp usamplerCube t_u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 fdims = uvec2(textureSize(t_f, int(min(1u, (v.metadata[0u].x - 1u)))));
  uvec2 idims = uvec2(textureSize(t_i, int(min(1u, (v.metadata[0u].y - 1u)))));
  uvec2 udims = uvec2(textureSize(t_u, int(min(1u, (v.metadata[0u].z - 1u)))));
}
