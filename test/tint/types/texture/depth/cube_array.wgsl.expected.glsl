#version 460

layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v;
uniform highp samplerCubeArray t_f;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = (v.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uvec2 dims = uvec2(textureSize(t_f, int(min(uint(0), v_1))).xy);
}
