#version 310 es
#extension GL_OES_sample_variables: require
precision highp float;
precision highp int;


struct FragIn {
  float a;
  uint mask;
};

layout(location = 0) in float tint_symbol_loc0_Input;
layout(location = 1) in float tint_symbol_loc1_Input;
layout(location = 0) out float tint_symbol_loc0_Output;
FragIn tint_symbol_inner(FragIn tint_symbol_1, float b) {
  if ((tint_symbol_1.mask == 0u)) {
    return tint_symbol_1;
  }
  return FragIn(b, 1u);
}
void main() {
  float v = tint_symbol_loc0_Input;
  FragIn v_1 = FragIn(v, uint(gl_SampleMaskIn[0u]));
  FragIn v_2 = tint_symbol_inner(v_1, tint_symbol_loc1_Input);
  tint_symbol_loc0_Output = v_2.a;
  gl_SampleMask[0u] = int(v_2.mask);
}
