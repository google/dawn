SKIP: FAILED

#version 310 es

struct main_out {
  float x_1_1;
};
precision highp float;
precision highp int;


float x_1 = 0.0f;
void main_1() {
}
main_out main() {
  main_1();
  return main_out(x_1);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
