SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

vec4 main(int loc0, uint loc1, float loc2, vec4 loc3, float16_t loc4, f16vec3 loc5) {
  int i = loc0;
  uint u = loc1;
  float f = loc2;
  vec4 v = loc3;
  float16_t x = loc4;
  f16vec3 y = loc5;
  return vec4(0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'main' : function cannot take any parameter(s) 
ERROR: 0:4: 'float' :  entry point cannot return a value
ERROR: 0:4: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
