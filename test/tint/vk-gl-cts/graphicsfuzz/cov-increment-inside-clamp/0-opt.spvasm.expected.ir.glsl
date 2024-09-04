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


uniform buf0 x_8;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a[3] = int[3](0, 0, 0);
  int b = 0;
  int c = 0;
  a[0] = 1;
  a[1] = 2;
  a[2] = 3;
  b = 0;
  c = a[x_8.one];
  if ((c > 1)) {
    x_GLF_color = vec4(0.0f, 1.0f, 1.0f, 0.0f);
    b = (b + 1);
  }
  int x_48 = (b + 1);
  b = x_48;
  int x_50_save = min(max(x_48, 0), 2);
  a[x_50_save] = (a[x_50_save] + 1);
  if ((a[2] == 4)) {
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
