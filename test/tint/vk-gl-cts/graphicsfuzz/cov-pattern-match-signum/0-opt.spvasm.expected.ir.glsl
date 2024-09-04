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


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
int func_i1_(inout int x) {
  if ((x_7.one == 1)) {
    int x_39 = x;
    return x_39;
  }
  int x_41 = x_7.one;
  return x_41;
}
void main_1() {
  int param = 0;
  param = -1;
  int x_28 = func_i1_(param);
  if ((x_28 <= 0)) {
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
