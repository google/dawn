SKIP: FAILED

#version 310 es

struct Communicators {
  float alice;
  vec4 bob;
};

struct main_out {
  vec4 x_2_1;
};

Communicators x_1 = Communicators(0.0f, vec4(0.0f));
vec4 x_2 = vec4(0.0f);
void main_1() {
}
main_out main(float x_1_param, vec4 x_1_param_1) {
  x_1.alice = x_1_param;
  x_1.bob = x_1_param_1;
  main_1();
  return main_out(x_2);
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'main' : function cannot take any parameter(s) 
ERROR: 0:16: 'structure' :  entry point cannot return a value
ERROR: 0:16: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
