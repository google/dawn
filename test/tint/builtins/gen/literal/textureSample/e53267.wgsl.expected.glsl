#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
uniform highp samplerCube f_arg_0_arg_1;
vec4 textureSample_e53267() {
  vec4 res = texture(f_arg_0_arg_1, vec3(1.0f));
  return res;
}
void main() {
  v.inner = textureSample_e53267();
}
