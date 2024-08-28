SKIP: FAILED

#version 310 es

void main_1() {
  bool a = false;
  int b = 0;
  uint c = 0u;
  float d = 0.0f;
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
