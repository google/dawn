#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

void dpdxCoarse_f64d7b() {
  vec3 res = dFdx(vec3(1.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdxCoarse_f64d7b();
}

void main() {
  fragment_main();
  return;
}
