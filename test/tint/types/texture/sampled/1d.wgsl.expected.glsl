#version 310 es

uniform highp sampler2D t_f_1;
uniform highp isampler2D t_i_1;
uniform highp usampler2D t_u_1;
void tint_symbol() {
  uint fdims = uvec2(textureSize(t_f_1, 1)).x;
  uint idims = uvec2(textureSize(t_i_1, 1)).x;
  uint udims = uvec2(textureSize(t_u_1, 1)).x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
