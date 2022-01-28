SKIP: FAILED

#version 310 es
precision mediump float;

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
  FragIn tint_symbol_3 = FragIn(a_1, uint(gl_SampleMask[0]));
  FragIn inner_result = tint_symbol(tint_symbol_3, b_1);
  a_2 = inner_result.a;
  gl_SampleMask_1[0] = inner_result.mask;
  return;
}
Error parsing GLSL shader:
ERROR: 0:21: 'gl_SampleMask' : required extension not requested: GL_OES_sample_variables
ERROR: 0:21: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



