SKIP: FAILED

#version 310 es

void main_1() {
  bool a = true;
  bool b = false;
  int c = -1;
  uint d = 1u;
  float e = 1.5f;
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
