#version 310 es
precision highp float;
precision highp int;

float fwidth_df38ef() {
  float res = fwidth(1.0f);
  return res;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void fragment_main() {
  prevent_dce.inner = fwidth_df38ef();
}

void main() {
  fragment_main();
  return;
}
