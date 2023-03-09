#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec2 inner;
} prevent_dce;

void dpdxCoarse_9581cf() {
  vec2 res = dFdx(vec2(1.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdxCoarse_9581cf();
}

void main() {
  fragment_main();
  return;
}
