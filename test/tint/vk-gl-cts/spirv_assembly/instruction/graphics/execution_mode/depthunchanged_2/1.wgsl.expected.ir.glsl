SKIP: FAILED

#version 310 es

struct main_out {
  vec4 outColor_1;
  float tint_symbol_1;
};
precision highp float;
precision highp int;


vec4 outColor = vec4(0.0f);
float tint_symbol = 0.0f;
void main_1() {
  outColor = vec4(0.0f);
  tint_symbol = 0.69999998807907104492f;
}
main_out main() {
  main_1();
  return main_out(outColor, tint_symbol);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
