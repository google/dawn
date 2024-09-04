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
  int x_45 = x_6.x_GLF_uniform_int_values[0].el;
  int x_48 = x_6.x_GLF_uniform_int_values[5].el;
  int x_51 = x_6.x_GLF_uniform_int_values[5].el;
  int x_54 = x_6.x_GLF_uniform_int_values[0].el;
  float v = float(x_45);
  float v_1 = float(x_48);
  float v_2 = float(x_51);
  x_GLF_color = vec4(v, v_1, v_2, float(x_54));
  int x_58 = x_6.x_GLF_uniform_int_values[1].el;
  int x_60 = x_6.x_GLF_uniform_int_values[2].el;
  int x_62 = x_6.x_GLF_uniform_int_values[3].el;
  int x_64 = x_6.x_GLF_uniform_int_values[4].el;
  int x_66 = x_6.x_GLF_uniform_int_values[0].el;
  data = int[5](x_58, x_60, x_62, x_64, x_66);
  int x_69 = x_6.x_GLF_uniform_int_values[5].el;
  a = x_69;
  {
    while(true) {
      int x_74 = a;
      int x_76 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_74 < x_76)) {
      } else {
        break;
      }
      int x_80 = x_6.x_GLF_uniform_int_values[5].el;
      i = x_80;
      {
        while(true) {
          int x_85 = i;
          int x_87 = x_6.x_GLF_uniform_int_values[1].el;
          if ((x_85 < x_87)) {
          } else {
            break;
          }
          int x_90 = i;
          j = x_90;
          {
            while(true) {
              int x_95 = j;
              int x_97 = x_6.x_GLF_uniform_int_values[1].el;
              if ((x_95 < x_97)) {
              } else {
                break;
              }
              int x_100 = i;
              int x_102 = data[x_100];
              int x_103 = j;
              int x_105 = data[x_103];
              if ((x_102 < x_105)) {
                int x_110 = x_6.x_GLF_uniform_int_values[5].el;
                float x_111 = float(x_110);
                x_GLF_color = vec4(x_111, x_111, x_111, x_111);
              }
              {
                int x_113 = j;
                j = (x_113 + 1);
              }
              continue;
            }
          }
          {
            int x_115 = i;
            i = (x_115 + 1);
          }
          continue;
        }
      }
      {
        int x_117 = a;
        a = (x_117 + 1);
      }
      continue;
    }
  }
  {
    while(true) {
      float x_124 = tint_symbol.x;
      float x_126 = x_12.x_GLF_uniform_float_values[0].el;
      if ((x_124 < x_126)) {
      } else {
        break;
      }
      int x_130 = x_6.x_GLF_uniform_int_values[5].el;
      i_1 = x_130;
      {
        while(true) {
          int x_135 = i_1;
          int x_137 = x_6.x_GLF_uniform_int_values[0].el;
          if ((x_135 < x_137)) {
          } else {
            break;
          }
          int x_141 = x_6.x_GLF_uniform_int_values[5].el;
          float x_142 = float(x_141);
          x_GLF_color = vec4(x_142, x_142, x_142, x_142);
          {
            int x_144 = i_1;
            i_1 = (x_144 + 1);
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
