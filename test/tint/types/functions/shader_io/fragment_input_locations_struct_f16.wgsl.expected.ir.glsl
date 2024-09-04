SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

#extension GL_AMD_gpu_shader_half_float: require

struct FragmentInputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  float16_t loc4;
  f16vec3 loc5;
};

void main(FragmentInputs inputs) {
  int i = inputs.loc0;
  uint u = inputs.loc1;
  float f = inputs.loc2;
  vec4 v = inputs.loc3;
  float16_t x = inputs.loc4;
  f16vec3 y = inputs.loc5;
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'main' : function cannot take any parameter(s) 
ERROR: 0:16: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
