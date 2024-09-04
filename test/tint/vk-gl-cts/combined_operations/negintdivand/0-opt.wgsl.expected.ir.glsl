SKIP: FAILED

#version 310 es

struct main_out {
  vec4 tint_symbol;
  vec4 frag_color_1;
};

vec4 pos = vec4(0.0f);
vec4 frag_color = vec4(0.0f);
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  vec4 x_21 = pos;
  tint_symbol = x_21;
  vec4 x_23 = pos;
  frag_color = (x_23 * 0.5f);
}
main_out main(vec4 position_param) {
  pos = position_param;
  main_1();
  return main_out(tint_symbol, frag_color);
}
error: Error parsing GLSL shader:
ERROR: 0:17: 'main' : function cannot take any parameter(s) 
ERROR: 0:17: 'structure' :  entry point cannot return a value
ERROR: 0:17: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
