SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf1 {
  strided_arr x_GLF_uniform_int_values[4];
};

struct strided_arr_1 {
  float el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_float_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf0 x_8;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float sums[2] = float[2](0.0f, 0.0f);
  int a = 0;
  int b = 0;
  int c = 0;
  int d = 0;
  mat2 indexable = mat2(vec2(0.0f), vec2(0.0f));
  mat2 indexable_1 = mat2(vec2(0.0f), vec2(0.0f));
  bool x_158 = false;
  bool x_159_phi = false;
  int x_16 = x_6.x_GLF_uniform_int_values[1].el;
  float x_85 = x_8.x_GLF_uniform_float_values[0].el;
  sums[x_16] = -(x_85);
  int x_18 = x_6.x_GLF_uniform_int_values[2].el;
  float x_90 = x_8.x_GLF_uniform_float_values[0].el;
  sums[x_18] = -(x_90);
  int x_19 = x_6.x_GLF_uniform_int_values[1].el;
  a = x_19;
  {
    while(true) {
      int x_20 = a;
      int x_21 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_20 < x_21)) {
      } else {
        break;
      }
      int x_22 = x_6.x_GLF_uniform_int_values[1].el;
      b = x_22;
      {
        while(true) {
          int x_23 = b;
          int x_24 = x_6.x_GLF_uniform_int_values[3].el;
          if ((x_23 < x_24)) {
          } else {
            break;
          }
          int x_25 = x_6.x_GLF_uniform_int_values[1].el;
          c = x_25;
          {
            while(true) {
              int x_26 = c;
              int x_27 = a;
              if ((x_26 <= x_27)) {
              } else {
                break;
              }
              int x_28 = x_6.x_GLF_uniform_int_values[1].el;
              d = x_28;
              {
                while(true) {
                  int x_29 = d;
                  int x_30 = x_6.x_GLF_uniform_int_values[3].el;
                  if ((x_29 < x_30)) {
                  } else {
                    break;
                  }
                  int x_31 = a;
                  int x_32 = x_6.x_GLF_uniform_int_values[2].el;
                  float x_125 = float(x_32);
                  int x_33 = c;
                  int x_34 = x_6.x_GLF_uniform_int_values[2].el;
                  vec2 v = vec2(x_125, 0.0f);
                  indexable = mat2(v, vec2(0.0f, x_125));
                  float x_131 = indexable[x_33][x_34];
                  sums[x_31] = x_131;
                  int x_35 = a;
                  int x_36 = x_6.x_GLF_uniform_int_values[2].el;
                  float x_134 = float(x_36);
                  int x_37 = c;
                  int x_38 = x_6.x_GLF_uniform_int_values[2].el;
                  vec2 v_1 = vec2(x_134, 0.0f);
                  indexable_1 = mat2(v_1, vec2(0.0f, x_134));
                  float x_140 = indexable_1[x_37][x_38];
                  float x_142 = sums[x_35];
                  sums[x_35] = (x_142 + x_140);
                  {
                    int x_39 = d;
                    d = (x_39 + 1);
                  }
                  continue;
                }
              }
              {
                int x_41 = c;
                c = (x_41 + 1);
              }
              continue;
            }
          }
          {
            int x_43 = b;
            b = (x_43 + 1);
          }
          continue;
        }
      }
      {
        int x_45 = a;
        a = (x_45 + 1);
      }
      continue;
    }
  }
  int x_47 = x_6.x_GLF_uniform_int_values[1].el;
  float x_147 = sums[x_47];
  float x_149 = x_8.x_GLF_uniform_float_values[1].el;
  bool x_150 = (x_147 == x_149);
  x_159_phi = x_150;
  if (x_150) {
    int x_48 = x_6.x_GLF_uniform_int_values[2].el;
    float x_155 = sums[x_48];
    float x_157 = x_8.x_GLF_uniform_float_values[2].el;
    x_158 = (x_155 == x_157);
    x_159_phi = x_158;
  }
  bool x_159 = x_159_phi;
  if (x_159) {
    int x_49 = x_6.x_GLF_uniform_int_values[2].el;
    int x_50 = x_6.x_GLF_uniform_int_values[1].el;
    int x_51 = x_6.x_GLF_uniform_int_values[1].el;
    int x_52 = x_6.x_GLF_uniform_int_values[2].el;
    float v_2 = float(x_49);
    float v_3 = float(x_50);
    float v_4 = float(x_51);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_52));
  } else {
    int x_53 = x_6.x_GLF_uniform_int_values[1].el;
    float x_173 = float(x_53);
    x_GLF_color = vec4(x_173, x_173, x_173, x_173);
  }
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
