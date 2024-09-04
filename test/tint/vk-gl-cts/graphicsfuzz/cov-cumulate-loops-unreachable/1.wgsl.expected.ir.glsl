SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  int b = 0;
  int i = 0;
  int i_1 = 0;
  int i_2 = 0;
  int indexable[2] = int[2](0, 0);
  a = 0;
  b = 1;
  x_GLF_color = vec4(0.0f);
  i = 0;
  {
    while(true) {
      int x_38 = i;
      if ((x_38 < 10)) {
      } else {
        break;
      }
      int x_41 = i;
      if ((x_41 > 1)) {
        int x_45 = a;
        a = (x_45 + 1);
        if (false) {
          i_1 = 0;
          {
            while(true) {
              int x_53 = i_1;
              if ((x_53 < 10)) {
              } else {
                break;
              }
              return;
            }
          }
        }
      }
      {
        int x_56 = i;
        i = (x_56 + 1);
      }
      continue;
    }
  }
  i_2 = 0;
  {
    while(true) {
      int x_62 = i_2;
      if ((x_62 < 10)) {
      } else {
        break;
      }
      int x_65 = b;
      indexable = int[2](1, 2);
      int x_67 = indexable[x_65];
      int x_68 = a;
      a = (x_68 + x_67);
      {
        int x_70 = i_2;
        i_2 = (x_70 + 1);
      }
      continue;
    }
  }
  int x_72 = a;
  if ((x_72 == 28)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
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
