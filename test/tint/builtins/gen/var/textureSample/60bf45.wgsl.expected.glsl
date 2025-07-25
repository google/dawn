#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DArrayShadow f_arg_0_arg_1;
float textureSample_60bf45() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  vec2 v_1 = arg_2;
  vec4 v_2 = vec4(v_1, float(arg_3), 0.0f);
  vec2 v_3 = dFdx(v_1);
  float res = textureGradOffset(f_arg_0_arg_1, v_2, v_3, dFdy(v_1), ivec2(1));
  return res;
}
void main() {
  v.inner = textureSample_60bf45();
}
