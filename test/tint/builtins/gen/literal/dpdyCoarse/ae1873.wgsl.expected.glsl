#version 310 es
precision highp float;
precision highp int;

vec3 dpdyCoarse_ae1873() {
  vec3 res = dFdy(vec3(1.0f));
  return res;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
  uint pad;
} prevent_dce;

void fragment_main() {
  prevent_dce.inner = dpdyCoarse_ae1873();
}

void main() {
  fragment_main();
  return;
}
