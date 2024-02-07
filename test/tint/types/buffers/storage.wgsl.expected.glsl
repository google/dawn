#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer weights_block_ssbo {
  float inner[];
} weights;

void tint_symbol() {
  float a = weights.inner[0];
}

void main() {
  tint_symbol();
  return;
}
