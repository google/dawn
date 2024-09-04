SKIP: FAILED

#version 310 es

struct buf0 {
  int two;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf0 x_7;
void main_1() {
  int a = 0;
  int i = 0;
  a = 0;
  i = 0;
  {
    while(true) {
      if ((i < 2)) {
      } else {
        break;
      }
      a = ((i | -2) - 1);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  if ((a == -2)) {
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
