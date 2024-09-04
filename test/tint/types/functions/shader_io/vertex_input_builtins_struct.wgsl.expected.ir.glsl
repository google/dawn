SKIP: FAILED

#version 310 es

struct VertexInputs {
  uint vertex_index;
  uint instance_index;
};

vec4 main(VertexInputs inputs) {
  uint foo = (inputs.vertex_index + inputs.instance_index);
  return vec4(0.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'main' : function cannot take any parameter(s) 
ERROR: 0:8: 'float' :  entry point cannot return a value
ERROR: 0:8: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
