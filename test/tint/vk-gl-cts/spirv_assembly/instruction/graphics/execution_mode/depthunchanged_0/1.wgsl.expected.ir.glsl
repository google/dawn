SKIP: FAILED

#version 310 es

struct main_out {
  vec4 outColor_1;
  float tint_symbol_2;
};
precision highp float;
precision highp int;


vec4 outColor = vec4(0.0f);
float tint_symbol = 0.0f;
vec4 tint_symbol_1 = vec4(0.0f);
void main_1() {
  outColor = vec4(0.0f);
  float x_20 = tint_symbol_1.z;
  tint_symbol = x_20;
}
main_out main(vec4 tint_symbol_4) {
  tint_symbol_1 = tint_symbol_4;
  main_1();
  return main_out(outColor, tint_symbol);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
