SKIP: FAILED

#version 310 es

struct VertexInputs0 {
  uint vertex_index;
  int loc0;
};

struct VertexInputs1 {
  uint loc1;
  vec4 loc3;
};

vec4 main(VertexInputs0 inputs0, uint loc1, uint instance_index, VertexInputs1 inputs1) {
  uint foo = (inputs0.vertex_index + instance_index);
  return vec4(0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'main' : function cannot take any parameter(s) 
ERROR: 0:13: 'float' :  entry point cannot return a value
ERROR: 0:13: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
