SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[3];
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


uniform buf1 x_7;
vec4 tint_symbol = vec4(0.0f);
uniform buf0 x_11;
vec4 x_GLF_color = vec4(0.0f);
bool continue_execution = true;
void main_1() {
  int a = 0;
  int i = 0;
  int j = 0;
  int x_36 = x_7.x_GLF_uniform_int_values[2].el;
  a = x_36;
  int x_38 = x_7.x_GLF_uniform_int_values[2].el;
  i = x_38;
  {
    while(true) {
      int x_43 = i;
      int x_45 = x_7.x_GLF_uniform_int_values[0].el;
      if ((x_43 < x_45)) {
      } else {
        break;
      }
      int x_49 = x_7.x_GLF_uniform_int_values[2].el;
      j = x_49;
      {
        while(true) {
          int x_54 = j;
          int x_56 = x_7.x_GLF_uniform_int_values[0].el;
          if ((x_54 < x_56)) {
          } else {
            break;
          }
          {
            while(true) {
              int x_64 = x_7.x_GLF_uniform_int_values[1].el;
              a = x_64;
              float x_66 = tint_symbol.y;
              float x_68 = x_11.x_GLF_uniform_float_values[0].el;
              if ((x_66 < x_68)) {
                continue_execution = false;
              }
              {
                int x_72 = a;
                int x_74 = x_7.x_GLF_uniform_int_values[1].el;
                if (!((x_72 < x_74))) { break; }
              }
              continue;
            }
          }
          float x_77 = tint_symbol.y;
          float x_79 = x_11.x_GLF_uniform_float_values[0].el;
          if ((x_77 < x_79)) {
            break;
          }
          {
            int x_83 = j;
            j = (x_83 + 1);
          }
          continue;
        }
      }
      {
        int x_85 = i;
        i = (x_85 + 1);
      }
      continue;
    }
  }
  int x_87 = a;
  int x_89 = x_7.x_GLF_uniform_int_values[1].el;
  if ((x_87 == x_89)) {
    int x_94 = a;
    int x_97 = x_7.x_GLF_uniform_int_values[2].el;
    int x_100 = x_7.x_GLF_uniform_int_values[2].el;
    int x_102 = a;
    float v = float(x_94);
    float v_1 = float(x_97);
    float v_2 = float(x_100);
    x_GLF_color = vec4(v, v_1, v_2, float(x_102));
  } else {
    int x_106 = x_7.x_GLF_uniform_int_values[2].el;
    float x_107 = float(x_106);
    x_GLF_color = vec4(x_107, x_107, x_107, x_107);
  }
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  main_out v_3 = main_out(x_GLF_color);
  if (!(continue_execution)) {
    discard;
  }
  return v_3;
}
error: Error parsing GLSL shader:
ERROR: 0:12: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
