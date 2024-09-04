SKIP: FAILED

#version 310 es

struct S {
  mat3 matrix;
  vec3 vector;
};
precision highp float;
precision highp int;


uniform S data;
void main() {
  vec3 x = (data.vector * data.matrix);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
