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
  int x_37 = x_7.x_GLF_uniform_int_values[0].el;
  i = x_37;
  {
    while(true) {
      int x_42 = i;
      int x_44 = x_7.x_GLF_uniform_int_values[1].el;
      if ((x_42 < x_44)) {
      } else {
        break;
      }
      int x_47 = i;
      switch(x_47) {
        case 2:
        {
          int x_83 = i;
          float x_85 = x_11.x_GLF_uniform_float_values[0].el;
          color[x_83] = x_85;
          break;
        }
        case 1:
        {
          int x_52 = x_7.x_GLF_uniform_int_values[0].el;
          j = x_52;
          {
            while(true) {
              int x_57 = i;
              int x_58 = i;
              if ((x_57 > x_58)) {
              } else {
                break;
              }
              int x_62 = x_7.x_GLF_uniform_int_values[0].el;
              k = x_62;
              {
                while(true) {
                  int x_67 = k;
                  int x_68 = i;
                  if ((x_67 < x_68)) {
                  } else {
                    break;
                  }
                  int x_71 = k;
                  float x_73 = x_11.x_GLF_uniform_float_values[0].el;
                  color[x_71] = x_73;
                  {
                    int x_75 = k;
                    k = (x_75 + 1);
                  }
                  continue;
                }
              }
              {
                int x_77 = j;
                j = (x_77 + 1);
              }
              continue;
            }
          }
          int x_79 = i;
          float x_81 = x_11.x_GLF_uniform_float_values[0].el;
          color[x_79] = x_81;
          break;
        }
        default:
        {
          break;
        }
      }
      {
        int x_87 = i;
        i = (x_87 + 1);
      }
      continue;
    }
  }
  vec4 x_89 = color;
  x_GLF_color = x_89;
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
