SKIP: FAILED

#version 310 es

struct VertexOutput {
  vec4 pos;
  int loc0;
};

VertexOutput foo(float x) {
  return VertexOutput(vec4(x, x, x, 1.0f), 42);
}
VertexOutput main() {
  return foo(0.5f);
}
VertexOutput main() {
  return foo(0.25f);
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'structure' :  entry point cannot return a value
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

struct VertexOutput {
  vec4 pos;
  int loc0;
};

VertexOutput foo(float x) {
  return VertexOutput(vec4(x, x, x, 1.0f), 42);
}
VertexOutput main() {
  return foo(0.5f);
}
VertexOutput main() {
  return foo(0.25f);
}
error: Error parsing GLSL shader:
ERROR: 0:11: 'structure' :  entry point cannot return a value
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
