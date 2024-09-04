SKIP: FAILED

#version 310 es

struct buf0 {
  int minusOne;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int minValue = 0;
  int negMinValue = 0;
  minValue = (-2147483647 - 1);
  int x_25 = minValue;
  negMinValue = -(x_25);
  int x_27 = negMinValue;
  int x_28 = minValue;
  int x_30 = x_7.minusOne;
  if ((x_27 == (x_28 * x_30))) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
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
