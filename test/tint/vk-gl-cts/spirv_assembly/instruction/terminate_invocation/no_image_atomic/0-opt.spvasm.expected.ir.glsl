SKIP: FAILED

#version 310 es

struct main_out {
  int x_4_1;
  vec4 tint_symbol;
};

vec3 x_2 = vec3(0.0f);
int x_3 = 0;
int x_4 = 0;
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  tint_symbol = vec4(x_2, 1.0f);
  x_4 = x_3;
}
main_out main(vec3 x_2_param, int x_3_param) {
  x_2 = x_2_param;
  x_3 = x_3_param;
  main_1();
  return main_out(x_4, tint_symbol);
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'main' : function cannot take any parameter(s) 
ERROR: 0:16: 'structure' :  entry point cannot return a value
ERROR: 0:16: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
