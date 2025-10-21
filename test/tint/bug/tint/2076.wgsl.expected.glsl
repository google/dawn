#version 310 es

uniform highp sampler2D randomTexture_plane0;
uniform highp sampler2D randomTexture_plane1;
layout(binding = 3, std140)
uniform randomTexture_params_block_1_ubo {
  uvec4 inner[17];
} v;
uniform highp sampler2D depthTexture;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
