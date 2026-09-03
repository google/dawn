#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DArrayShadow f_arg_0_arg_1;
float textureSample_4703d0() {
  vec2 v_1 = dFdx(vec2(1.0f));
  float res = textureGradOffset(f_arg_0_arg_1, vec4(1.0f, 1.0f, 1.0f, 0.0f), v_1, dFdy(vec2(1.0f)), ivec2(1));
  return res;
}
void main() {
  v.inner = textureSample_4703d0();
}
