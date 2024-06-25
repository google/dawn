#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

vec4 fwidthFine_68f4ef() {
  vec4 res = fwidth(vec4(1.0f));
  return res;
}

void fragment_main() {
  prevent_dce.inner = fwidthFine_68f4ef();
}

void main() {
  fragment_main();
  return;
}
