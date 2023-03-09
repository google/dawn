#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void fwidthFine_68f4ef() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = fwidth(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  fwidthFine_68f4ef();
}

void main() {
  fragment_main();
  return;
}
