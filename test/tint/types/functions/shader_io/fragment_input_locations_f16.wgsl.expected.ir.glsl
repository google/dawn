SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

#extension GL_AMD_gpu_shader_half_float: require

void main(int loc0, uint loc1, float loc2, vec4 loc3, float16_t loc4, f16vec3 loc5) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  vec4 v = loc3;
  float16_t x = loc4;
  f16vec3 y = loc5;
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'main' : function cannot take any parameter(s) 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
