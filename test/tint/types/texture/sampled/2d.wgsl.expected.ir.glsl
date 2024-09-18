#version 310 es

uniform highp sampler2D t_f;
uniform highp isampler2D t_i;
uniform highp usampler2D t_u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 fdims = uvec2(textureSize(t_f, 1));
  uvec2 idims = uvec2(textureSize(t_i, 1));
  uvec2 udims = uvec2(textureSize(t_u, 1));
}
