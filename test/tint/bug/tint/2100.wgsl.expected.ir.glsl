SKIP: FAILED

#version 310 es

struct S {
  mat4 matrix_view;
  mat3 matrix_normal;
};

uniform S tint_symbol;
vec4 main() {
  float x = tint_symbol.matrix_view[0].z;
  return vec4(x, 0.0f, 0.0f, 1.0f);
}
error: Error parsing GLSL shader:
ERROR: 0:9: 'float' :  entry point cannot return a value
ERROR: 0:9: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
