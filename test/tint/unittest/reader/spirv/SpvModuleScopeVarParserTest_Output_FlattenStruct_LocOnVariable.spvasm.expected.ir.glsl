SKIP: FAILED

#version 310 es

struct Communicators {
  float alice;
  vec4 bob;
};

struct main_out {
  float x_1_1;
  vec4 x_1_2;
  vec4 x_2_1;
};

Communicators x_1 = Communicators(0.0f, vec4(0.0f));
vec4 x_2 = vec4(0.0f);
void main_1() {
}
main_out main() {
  main_1();
  return main_out(x_1.alice, x_1.bob, x_2);
}
error: Error parsing GLSL shader:
ERROR: 0:18: 'structure' :  entry point cannot return a value
ERROR: 0:18: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
