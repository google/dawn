SKIP: FAILED

#version 310 es

float x_200() {
  return 0.0f;
}
void x_100_1() {
}
void main() {
  x_100_1();
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
