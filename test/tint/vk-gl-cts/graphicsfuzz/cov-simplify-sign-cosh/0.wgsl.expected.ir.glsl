SKIP: FAILED

#version 310 es

struct buf0 {
  int one;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_5;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  bool x_31 = false;
  bool x_32_phi = false;
  bool x_26 = true;
  x_32_phi = x_26;
  if (!(x_26)) {
    int x_6 = x_5.one;
    x_31 = (x_6 == 1);
    x_32_phi = x_31;
  }
  bool x_32 = x_32_phi;
  if (x_32) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(1.0f);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
