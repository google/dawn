#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DShadow f_arg_0_arg_1;
float textureSampleCompare_3a5923() {
  float res = texture(f_arg_0_arg_1, vec3(vec2(1.0f), 1.0f));
  return res;
}
void main() {
  v.inner = textureSampleCompare_3a5923();
}
