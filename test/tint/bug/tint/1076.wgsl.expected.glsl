#version 310 es
#extension GL_OES_sample_variables : require
precision highp float;

layout(location = 0) in float a_1;
layout(location = 1) in float b_1;
layout(location = 0) out float a_2;
struct FragIn {
  float a;
  uint mask;
};

FragIn tint_symbol(FragIn tint_symbol_1, float b) {
  if ((tint_symbol_1.mask == 0u)) {
    return tint_symbol_1;
  }
  FragIn tint_symbol_2 = FragIn(b, 1u);
  return tint_symbol_2;
}

void main() {
  FragIn tint_symbol_3 = FragIn(a_1, uint(gl_SampleMaskIn[0]));
  FragIn inner_result = tint_symbol(tint_symbol_3, b_1);
  a_2 = inner_result.a;
  gl_SampleMask[0] = int(inner_result.mask);
  return;
}
