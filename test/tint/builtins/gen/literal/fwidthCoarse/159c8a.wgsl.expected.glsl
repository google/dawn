#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void fwidthCoarse_159c8a() {
  float res = fwidth(1.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  fwidthCoarse_159c8a();
}

void main() {
  fragment_main();
  return;
}
