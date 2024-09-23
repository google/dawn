SKIP: FAILED

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
  FragIn v = FragIn(tint_symbol_loc0_Input, gl_SampleMaskIn);
  FragIn v_1 = tint_symbol_inner(v, tint_symbol_loc1_Input);
  tint_symbol_loc0_Output = v_1.a;
  gl_SampleMask = v_1.mask;
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'constructor' : array argument must be sized 
ERROR: 0:22: '=' :  cannot convert from ' const float' to ' temp structure{ global highp float a,  global highp uint mask}'
ERROR: 0:22: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
