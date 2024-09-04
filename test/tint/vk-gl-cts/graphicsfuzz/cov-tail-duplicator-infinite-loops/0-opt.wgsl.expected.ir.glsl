SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct buf2 {
  float zero;
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[2];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_5;
uniform buf2 x_7;
uniform buf0 x_10;
void main_1() {
  int i = 0;
  float x_38 = x_5.x_GLF_uniform_float_values[0].el;
  x_GLF_color = vec4(x_38, x_38, x_38, x_38);
  float x_41 = x_7.zero;
  float x_43 = x_5.x_GLF_uniform_float_values[0].el;
  if ((x_41 > x_43)) {
    {
      while(true) {
        float x_53 = x_5.x_GLF_uniform_float_values[1].el;
        x_GLF_color = vec4(x_53, x_53, x_53, x_53);
        {
          if (false) { break; }
        }
        continue;
      }
    }
  } else {
    {
      while(true) {
        {
          while(true) {
            if (true) {
            } else {
              break;
            }
            int x_13 = x_10.x_GLF_uniform_int_values[1].el;
            i = x_13;
            {
              while(true) {
                int x_14 = i;
                int x_15 = x_10.x_GLF_uniform_int_values[0].el;
                if ((x_14 < x_15)) {
                } else {
                  break;
                }
                float x_73 = x_5.x_GLF_uniform_float_values[1].el;
                float x_75 = x_5.x_GLF_uniform_float_values[0].el;
                float x_77 = x_5.x_GLF_uniform_float_values[0].el;
                float x_79 = x_5.x_GLF_uniform_float_values[1].el;
                x_GLF_color = vec4(x_73, x_75, x_77, x_79);
                {
                  int x_16 = i;
                  i = (x_16 + 1);
                }
                continue;
              }
            }
            break;
          }
        }
        {
          float x_82 = x_7.zero;
          float x_84 = x_5.x_GLF_uniform_float_values[0].el;
          if (!((x_82 > x_84))) { break; }
        }
        continue;
      }
    }
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
