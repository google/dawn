SKIP: FAILED

#version 310 es

void main_1() {
  uint a = 0u;
  int b = 0;
  float c = 0.0f;
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
