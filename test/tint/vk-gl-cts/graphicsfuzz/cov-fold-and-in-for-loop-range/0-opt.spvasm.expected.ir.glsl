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
int func_() {
  int ret = 0;
  int i = 0;
  ret = 0;
  i = 3;
  {
    while(true) {
      if ((i > (i & 1))) {
      } else {
        break;
      }
      ret = (ret + 1);
      {
        i = (i - x_8.one);
      }
      continue;
    }
  }
  int x_50 = ret;
  return x_50;
}
void main_1() {
  int x_29 = func_();
  if ((x_29 == 2)) {
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
