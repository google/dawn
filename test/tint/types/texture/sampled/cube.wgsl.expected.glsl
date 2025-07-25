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
  uint v_1 = (v.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uvec2 fdims = uvec2(textureSize(t_f, int(min(uint(1), v_1))));
  uint v_2 = (v.metadata[(1u / 4u)][(1u % 4u)] - 1u);
  uvec2 idims = uvec2(textureSize(t_i, int(min(uint(1), v_2))));
  uint v_3 = (v.metadata[(2u / 4u)][(2u % 4u)] - 1u);
  uvec2 udims = uvec2(textureSize(t_u, int(min(uint(1), v_3))));
}
