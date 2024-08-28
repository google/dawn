SKIP: FAILED

#version 310 es

bool x_1 = true;
bool x_2 = false;
int x_3 = -1;
uint x_4 = 1u;
float x_5 = 1.5f;
void main_1() {
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:7: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:7: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
