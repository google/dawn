#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer weights_block_1_ssbo {
  float inner[];
} v;
void main() {
  float a = v.inner[0];
}
