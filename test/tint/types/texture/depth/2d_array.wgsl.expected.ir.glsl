#version 310 es

uniform highp sampler2DArray t_f;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec2 dims = uvec2(textureSize(t_f, 0).xy);
}
