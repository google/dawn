#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

void fwidthCoarse_1e59d9() {
  vec3 arg_0 = vec3(1.0f);
  vec3 res = fwidth(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  fwidthCoarse_1e59d9();
}

void main() {
  fragment_main();
  return;
}
