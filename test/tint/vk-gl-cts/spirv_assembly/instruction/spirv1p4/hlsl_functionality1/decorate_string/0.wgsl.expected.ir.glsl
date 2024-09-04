SKIP: FAILED

#version 310 es

struct main_out {
  vec4 tint_symbol;
  uint pos_1;
};

vec4 vert_pos = vec4(0.0f);
uint pos = 0u;
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  vec4 x_22 = vert_pos;
  tint_symbol = x_22;
  pos = 0u;
}
main_out main(vec4 position_param) {
  vert_pos = position_param;
  main_1();
  return main_out(tint_symbol, pos);
}
error: Error parsing GLSL shader:
ERROR: 0:16: 'main' : function cannot take any parameter(s) 
ERROR: 0:16: 'structure' :  entry point cannot return a value
ERROR: 0:16: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
