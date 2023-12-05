#version 310 es

uniform highp samplerCube t_f_1;
uniform highp isamplerCube t_i_1;
uniform highp usamplerCube t_u_1;
void tint_symbol() {
  uvec2 fdims = uvec2(textureSize(t_f_1, 1));
  uvec2 idims = uvec2(textureSize(t_i_1, 1));
  uvec2 udims = uvec2(textureSize(t_u_1, 1));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
