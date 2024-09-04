SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[6];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[1];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 x_GLF_color = vec4(0.0f);
uniform buf1 x_6;
vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_12;
void main_1() {
  int data[5] = int[5](0, 0, 0, 0, 0);
  int a = 0;
  int i = 0;
  int j = 0;
  int i_1 = 0;
  float v = float(x_6.x_GLF_uniform_int_values[0].el);
  float v_1 = float(x_6.x_GLF_uniform_int_values[5].el);
  float v_2 = float(x_6.x_GLF_uniform_int_values[5].el);
  x_GLF_color = vec4(v, v_1, v_2, float(x_6.x_GLF_uniform_int_values[0].el));
  data = int[5](x_6.x_GLF_uniform_int_values[1].el, x_6.x_GLF_uniform_int_values[2].el, x_6.x_GLF_uniform_int_values[3].el, x_6.x_GLF_uniform_int_values[4].el, x_6.x_GLF_uniform_int_values[0].el);
  a = x_6.x_GLF_uniform_int_values[5].el;
  {
    while(true) {
      if ((a < x_6.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      i = x_6.x_GLF_uniform_int_values[5].el;
      {
        while(true) {
          if ((i < x_6.x_GLF_uniform_int_values[1].el)) {
          } else {
            break;
          }
          j = i;
          {
            while(true) {
              if ((j < x_6.x_GLF_uniform_int_values[1].el)) {
              } else {
                break;
              }
              if ((data[i] < data[j])) {
                x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[5].el));
              }
              {
                j = (j + 1);
              }
              continue;
            }
          }
          {
            i = (i + 1);
          }
          continue;
        }
      }
      {
        a = (a + 1);
      }
      continue;
    }
  }
  {
    while(true) {
      if ((tint_symbol.x < x_12.x_GLF_uniform_float_values[0].el)) {
      } else {
        break;
      }
      i_1 = x_6.x_GLF_uniform_int_values[5].el;
      {
        while(true) {
          if ((i_1 < x_6.x_GLF_uniform_int_values[0].el)) {
          } else {
            break;
          }
          x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[5].el));
          {
            i_1 = (i_1 + 1);
          }
          continue;
        }
      }
      {
      }
      continue;
    }
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
