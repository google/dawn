SKIP: FAILED

#version 310 es
#extension GL_OES_sample_variables: require
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct FragmentOutputs {
  int loc0;
  float frag_depth;
  uint loc1;
  float loc2;
  uint sample_mask;
  vec4 loc3;
  float16_t loc4;
  f16vec3 loc5;
};

layout(location = 0) out int tint_symbol_loc0_Output;
layout(location = 1) out uint tint_symbol_loc1_Output;
layout(location = 2) out float tint_symbol_loc2_Output;
layout(location = 3) out vec4 tint_symbol_loc3_Output;
layout(location = 4) out float16_t tint_symbol_loc4_Output;
layout(location = 5) out f16vec3 tint_symbol_loc5_Output;
FragmentOutputs tint_symbol_inner() {
  return FragmentOutputs(1, 2.0f, 1u, 1.0f, 2u, vec4(1.0f, 2.0f, 3.0f, 4.0f), 2.25hf, f16vec3(3.0hf, 5.0hf, 8.0hf));
}
void main() {
  FragmentOutputs v = tint_symbol_inner();
  tint_symbol_loc0_Output = v.loc0;
  gl_FragDepth = v.frag_depth;
  tint_symbol_loc1_Output = v.loc1;
  tint_symbol_loc2_Output = v.loc2;
  gl_SampleMask = v.sample_mask;
  tint_symbol_loc3_Output = v.loc3;
  tint_symbol_loc4_Output = v.loc4;
  tint_symbol_loc5_Output = v.loc5;
}
error: Error parsing GLSL shader:
ERROR: 0:34: 'assign' :  cannot convert from ' global highp uint' to ' out unsized 1-element array of highp int SampleMaskIn'
ERROR: 0:34: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
