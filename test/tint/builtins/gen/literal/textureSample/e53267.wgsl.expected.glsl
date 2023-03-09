#version 310 es
precision highp float;

uniform highp samplerCube arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSample_e53267() {
  vec4 res = texture(arg_0_arg_1, vec3(1.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_e53267();
}

void main() {
  fragment_main();
  return;
}
