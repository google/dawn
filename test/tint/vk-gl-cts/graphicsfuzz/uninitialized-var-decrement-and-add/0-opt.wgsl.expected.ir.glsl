SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
vec4 tint_symbol = vec4(0.0f);
void main_1() {
  float x_30 = 0.0f;
  uint foo = 0u;
  x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  float x_32 = tint_symbol.x;
  if ((x_32 > -1.0f)) {
    float x_38 = x_GLF_color.x;
    x_30 = x_38;
  } else {
    uint x_6 = foo;
    uint x_7 = (x_6 - 1u);
    foo = x_7;
    x_30 = float((178493u + x_7));
  }
  float x_40 = x_30;
  x_GLF_color[0u] = x_40;
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
