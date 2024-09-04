SKIP: FAILED

#version 310 es

struct main_out {
  vec4 tint_symbol;
  vec4 frag_color_1;
};

vec4 position_1 = vec4(0.0f);
vec4 frag_color = vec4(0.0f);
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  tint_symbol = position_1;
  frag_color = (position_1 * 0.5f);
}
main_out main(vec4 position_1_param) {
  position_1 = position_1_param;
  main_1();
  return main_out(tint_symbol, frag_color);
}
error: Error parsing GLSL shader:
ERROR: 0:15: 'main' : function cannot take any parameter(s) 
ERROR: 0:15: 'structure' :  entry point cannot return a value
ERROR: 0:15: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
