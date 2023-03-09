#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void fwidthCoarse_4e4fc4() {
  vec4 res = fwidth(vec4(1.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  fwidthCoarse_4e4fc4();
}

void main() {
  fragment_main();
  return;
}
