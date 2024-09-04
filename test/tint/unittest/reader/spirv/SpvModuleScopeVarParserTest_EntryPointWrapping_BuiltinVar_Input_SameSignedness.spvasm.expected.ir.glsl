SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_4_1;
};

uint x_1 = 0u;
vec4 x_4 = vec4(0.0f);
void main_1() {
  uint x_2 = x_1;
}
main_out main(uint x_1_param) {
  x_1 = x_1_param;
  main_1();
  return main_out(x_4);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'main' : function cannot take any parameter(s) 
ERROR: 0:12: 'structure' :  entry point cannot return a value
ERROR: 0:12: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
