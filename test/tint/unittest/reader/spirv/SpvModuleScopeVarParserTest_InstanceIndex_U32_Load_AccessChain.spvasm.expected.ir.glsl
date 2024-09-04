SKIP: FAILED

#version 310 es

struct main_out {
  vec4 position_1_1;
};

uint x_4 = 0u;
vec4 position_1 = vec4(0.0f);
void main_1() {
  uint x_2 = x_4;
}
main_out main(uint x_4_param) {
  x_4 = x_4_param;
  main_1();
  return main_out(position_1);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'main' : function cannot take any parameter(s) 
ERROR: 0:12: 'structure' :  entry point cannot return a value
ERROR: 0:12: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
