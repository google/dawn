SKIP: FAILED

#version 310 es

struct VertexOutputs {
  vec4 position;
};

VertexOutputs main() {
  return VertexOutputs(vec4(1.0f, 2.0f, 3.0f, 4.0f));
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'structure' :  entry point cannot return a value
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
