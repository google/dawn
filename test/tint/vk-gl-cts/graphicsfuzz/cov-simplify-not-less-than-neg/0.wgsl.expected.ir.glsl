SKIP: FAILED

#version 310 es

struct buf0 {
  int three;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  bool x_36 = false;
  bool x_37_phi = false;
  int x_29 = x_6.three;
  bool x_30 = (x_29 > 1);
  x_37_phi = x_30;
  if (x_30) {
    float x_34 = tint_symbol.y;
    x_36 = !((x_34 < -5.0f));
    x_37_phi = x_36;
  }
  bool x_37 = x_37_phi;
  if (x_37) {
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
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
