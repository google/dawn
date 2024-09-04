SKIP: FAILED

#version 310 es

struct VertexInputs0 {
  uint vertex_index;
  int loc0;
};
#extension GL_AMD_gpu_shader_half_float: require

struct VertexInputs1 {
  float loc2;
  vec4 loc3;
  f16vec3 loc5;
};

vec4 main(VertexInputs0 inputs0, uint loc1, uint instance_index, VertexInputs1 inputs1, float16_t loc4) {
  uint foo = (inputs0.vertex_index + instance_index);
  int i = inputs0.loc0;
  uint u = loc1;
  float f = inputs1.loc2;
  vec4 v = inputs1.loc3;
  float16_t x = loc4;
  f16vec3 y = inputs1.loc5;
  return vec4(0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'main' : function cannot take any parameter(s) 
ERROR: 0:15: 'float' :  entry point cannot return a value
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
