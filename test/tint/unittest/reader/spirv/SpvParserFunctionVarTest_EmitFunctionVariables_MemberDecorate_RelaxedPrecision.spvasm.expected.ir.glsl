SKIP: FAILED

#version 310 es

struct strct {
  float field0;
};

void main_1() {
  strct myvar = strct(0.0f);
}
void main() {
  main_1();
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
