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


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  int i = 0;
  a = 0;
  i = 0;
  {
    while(true) {
      int x_31 = i;
      int x_33 = x_7.three;
      if ((x_31 < (7 + x_33))) {
      } else {
        break;
      }
      int x_37 = i;
      switch(x_37) {
        case 7:
        case 8:
        {
          int x_40 = a;
          a = (x_40 + 1);
          break;
        }
        default:
        {
          break;
        }
      }
      {
        int x_42 = i;
        i = (x_42 + 1);
      }
      continue;
    }
  }
  int x_44 = a;
  if ((x_44 == 2)) {
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
