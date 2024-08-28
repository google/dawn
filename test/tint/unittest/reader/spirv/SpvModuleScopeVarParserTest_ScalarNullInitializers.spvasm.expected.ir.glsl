SKIP: FAILED

#version 310 es

bool x_1 = false;
int x_2 = 0;
uint x_3 = 0u;
float x_4 = 0.0f;
void main_1() {
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:6: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
