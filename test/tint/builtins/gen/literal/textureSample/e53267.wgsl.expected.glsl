#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp samplerCube arg_0_arg_1;

vec4 textureSample_e53267() {
  vec4 res = texture(arg_0_arg_1, vec3(1.0f));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSample_e53267();
}

void main() {
  fragment_main();
  return;
}
