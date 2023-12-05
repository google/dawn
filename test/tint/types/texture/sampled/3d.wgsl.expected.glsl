#version 310 es

uniform highp sampler3D t_f_1;
uniform highp isampler3D t_i_1;
uniform highp usampler3D t_u_1;
void tint_symbol() {
  uvec3 fdims = uvec3(textureSize(t_f_1, 1));
  uvec3 idims = uvec3(textureSize(t_i_1, 1));
  uvec3 udims = uvec3(textureSize(t_u_1, 1));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
