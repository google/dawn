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
      if ((i < 10)) {
      } else {
        break;
      }
      if ((i > 1)) {
        a = (a + 1);
        if (false) {
          i_1 = 0;
          {
            while(true) {
              if ((i_1 < 10)) {
              } else {
                break;
              }
              return;
            }
          }
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  i_2 = 0;
  {
    while(true) {
      if ((i_2 < 10)) {
      } else {
        break;
      }
      int x_65 = b;
      indexable = int[2](1, 2);
      a = (a + indexable[x_65]);
      {
        i_2 = (i_2 + 1);
      }
      continue;
    }
  }
  if ((a == 28)) {
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
