SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[2];
};

struct strided_arr_1 {
  float el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_7;
uniform buf1 x_11;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 color = vec4(0.0f);
  int i = 0;
  int j = 0;
  int k = 0;
  color = vec4(1.0f);
  i = x_7.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i < x_7.x_GLF_uniform_int_values[1].el)) {
      } else {
        break;
      }
      int x_47 = i;
      switch(x_47) {
        case 2:
        {
          int x_83 = i;
          color[x_83] = x_11.x_GLF_uniform_float_values[0].el;
          break;
        }
        case 1:
        {
          j = x_7.x_GLF_uniform_int_values[0].el;
          {
            while(true) {
              if ((i > i)) {
              } else {
                break;
              }
              k = x_7.x_GLF_uniform_int_values[0].el;
              {
                while(true) {
                  if ((k < i)) {
                  } else {
                    break;
                  }
                  int x_71 = k;
                  color[x_71] = x_11.x_GLF_uniform_float_values[0].el;
                  {
                    k = (k + 1);
                  }
                  continue;
                }
              }
              {
                j = (j + 1);
              }
              continue;
            }
          }
          int x_79 = i;
          color[x_79] = x_11.x_GLF_uniform_float_values[0].el;
          break;
        }
        default:
        {
          break;
        }
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  x_GLF_color = color;
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
