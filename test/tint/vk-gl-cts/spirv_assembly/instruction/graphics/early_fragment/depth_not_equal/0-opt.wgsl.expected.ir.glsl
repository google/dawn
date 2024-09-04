SKIP: FAILED

#version 310 es

struct main_out {
  vec4 tint_symbol;
};

vec4 pos = vec4(0.0f);
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  vec4 x_22 = pos;
  vec2 x_23 = vec2(x_22[0u], x_22[1u]);
  tint_symbol = vec4(x_23[0u], x_23[1u], 0.30000001192092895508f, 1.0f);
}
main_out main(vec4 position_param) {
  pos = position_param;
  main_1();
  return main_out(tint_symbol);
}
error: Error parsing GLSL shader:
ERROR: 0:14: 'main' : function cannot take any parameter(s) 
ERROR: 0:14: 'structure' :  entry point cannot return a value
ERROR: 0:14: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
