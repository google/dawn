#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec2 inner;
} prevent_dce;

vec2 dpdxCoarse_9581cf() {
  vec2 res = dFdx(vec2(1.0f));
  return res;
}

void fragment_main() {
  prevent_dce.inner = dpdxCoarse_9581cf();
}

void main() {
  fragment_main();
  return;
}
