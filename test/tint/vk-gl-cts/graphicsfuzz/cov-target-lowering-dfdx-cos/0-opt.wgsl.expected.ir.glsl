SKIP: FAILED

#version 310 es

struct buf0 {
  float two;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_8;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  float x_33 = tint_symbol.x;
  a = dFdx(cos(x_33));
  float x_37 = x_8.two;
  float x_38 = a;
  b = mix(2.0f, x_37, x_38);
  float x_40 = b;
  float x_42 = b;
  if (((x_40 >= 1.89999997615814208984f) & (x_42 <= 2.09999990463256835938f))) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
  }
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
