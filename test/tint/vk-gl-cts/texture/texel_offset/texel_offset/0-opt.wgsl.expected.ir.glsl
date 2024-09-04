SKIP: FAILED

#version 310 es

struct main_out {
  vec4 result_1;
};
precision highp float;
precision highp int;


vec4 result = vec4(0.0f);
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  float x_19 = tint_symbol.x;
  float x_23 = tint_symbol.y;
  float v = (floor(x_19) / 255.0f);
  result = vec4(v, (floor(x_23) / 255.0f), 0.0f, 0.0f);
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(result);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
