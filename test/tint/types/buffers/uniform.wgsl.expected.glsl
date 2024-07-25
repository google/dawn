#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std140) uniform weights_block_ubo {
  vec2 inner;
} weights;

void tint_symbol() {
  float a = weights.inner[0];
}

void main() {
  tint_symbol();
  return;
}
