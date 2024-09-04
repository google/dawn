SKIP: FAILED

#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct VertexInputs {
  int loc0;
  uint loc1;
  float loc2;
  vec4 loc3;
  float16_t loc4;
  f16vec3 loc5;
};

vec4 main(VertexInputs inputs) {
  int i = inputs.loc0;
  uint u = inputs.loc1;
  float f = inputs.loc2;
  vec4 v = inputs.loc3;
  float16_t x = inputs.loc4;
  f16vec3 y = inputs.loc5;
  return vec4(0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'main' : function cannot take any parameter(s) 
ERROR: 0:13: 'float' :  entry point cannot return a value
ERROR: 0:13: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
