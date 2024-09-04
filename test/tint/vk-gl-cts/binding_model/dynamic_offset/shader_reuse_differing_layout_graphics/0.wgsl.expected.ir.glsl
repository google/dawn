SKIP: FAILED

#version 310 es

struct block0 {
  vec4 in_color;
};

struct main_out {
  vec4 tint_symbol;
  vec4 frag_color_1;
};

vec4 pos = vec4(0.0f);
vec4 frag_color = vec4(0.0f);
uniform block0 x_8;
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  vec4 x_24 = pos;
  tint_symbol = x_24;
  vec4 x_27 = x_8.in_color;
  frag_color = x_27;
}
main_out main(vec4 position_param) {
  pos = position_param;
  main_1();
  return main_out(tint_symbol, frag_color);
}
error: Error parsing GLSL shader:
ERROR: 0:22: 'main' : function cannot take any parameter(s) 
ERROR: 0:22: 'structure' :  entry point cannot return a value
ERROR: 0:22: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.




tint executable returned error: exit status 1
