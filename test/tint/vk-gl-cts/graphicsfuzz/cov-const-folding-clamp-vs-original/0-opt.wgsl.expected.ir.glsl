SKIP: FAILED

#version 310 es

struct buf0 {
  float one;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_6;
void main_1() {
  float f = 0.0f;
  x_GLF_color = vec4(0.0f);
  float x_23 = x_6.one;
  f = clamp(x_23, 1.0f, 1.0f);
  float x_25 = f;
  float x_27 = x_6.one;
  if ((x_25 > x_27)) {
    x_GLF_color = vec4(0.0f);
  } else {
    float x_32 = f;
    x_GLF_color = vec4(x_32, 0.0f, 0.0f, 1.0f);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
