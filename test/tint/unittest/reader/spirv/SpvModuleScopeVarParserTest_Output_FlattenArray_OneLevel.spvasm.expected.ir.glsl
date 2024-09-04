SKIP: FAILED

#version 310 es

struct main_out {
  float x_1_1;
  float x_1_2;
  float x_1_3;
  vec4 x_2_1;
};

float x_1[3] = float[3](0.0f, 0.0f, 0.0f);
vec4 x_2 = vec4(0.0f);
void main_1() {
}
main_out main() {
  main_1();
  return main_out(x_1[0], x_1[1], x_1[2], x_2);
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'structure' :  entry point cannot return a value
ERROR: 0:14: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
