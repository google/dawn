#version 310 es

uniform highp sampler3D t_f;
uniform highp isampler3D t_i;
uniform highp usampler3D t_u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec3 fdims = uvec3(textureSize(t_f, 1));
  uvec3 idims = uvec3(textureSize(t_i, 1));
  uvec3 udims = uvec3(textureSize(t_u, 1));
}
