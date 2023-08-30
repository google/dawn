#version 310 es

struct Results {
  float colorSamples[4];
};

layout(binding = 2, std430) buffer results_block_ssbo {
  Results inner;
} results;

uniform highp sampler2DMS texture0_1;
void tint_symbol() {
  results.inner.colorSamples[0] = texelFetch(texture0_1, ivec2(0), 0).x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
