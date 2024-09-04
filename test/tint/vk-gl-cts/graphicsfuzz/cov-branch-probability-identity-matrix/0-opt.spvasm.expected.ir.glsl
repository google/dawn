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
  bool x_159 = false;
  int x_16 = x_6.x_GLF_uniform_int_values[1].el;
  sums[x_16] = -(x_8.x_GLF_uniform_float_values[0].el);
  int x_18 = x_6.x_GLF_uniform_int_values[2].el;
  sums[x_18] = -(x_8.x_GLF_uniform_float_values[0].el);
  a = x_6.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((a < x_6.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      b = x_6.x_GLF_uniform_int_values[1].el;
      {
        while(true) {
          if ((b < x_6.x_GLF_uniform_int_values[3].el)) {
          } else {
            break;
          }
          c = x_6.x_GLF_uniform_int_values[1].el;
          {
            while(true) {
              if ((c <= a)) {
              } else {
                break;
              }
              d = x_6.x_GLF_uniform_int_values[1].el;
              {
                while(true) {
                  if ((d < x_6.x_GLF_uniform_int_values[3].el)) {
                  } else {
                    break;
                  }
                  int x_31 = a;
                  float x_125 = float(x_6.x_GLF_uniform_int_values[2].el);
                  int x_33 = c;
                  int x_34 = x_6.x_GLF_uniform_int_values[2].el;
                  vec2 v = vec2(x_125, 0.0f);
                  indexable = mat2(v, vec2(0.0f, x_125));
                  sums[x_31] = indexable[x_33][x_34];
                  int x_35 = a;
                  float x_134 = float(x_6.x_GLF_uniform_int_values[2].el);
                  int x_37 = c;
                  int x_38 = x_6.x_GLF_uniform_int_values[2].el;
                  vec2 v_1 = vec2(x_134, 0.0f);
                  indexable_1 = mat2(v_1, vec2(0.0f, x_134));
                  sums[x_35] = (sums[x_35] + indexable_1[x_37][x_38]);
                  {
                    d = (d + 1);
                  }
                  continue;
                }
              }
              {
                c = (c + 1);
              }
              continue;
            }
          }
          {
            b = (b + 1);
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
  bool x_150 = (sums[x_6.x_GLF_uniform_int_values[1].el] == x_8.x_GLF_uniform_float_values[1].el);
  x_159 = x_150;
  if (x_150) {
    x_158 = (sums[x_6.x_GLF_uniform_int_values[2].el] == x_8.x_GLF_uniform_float_values[2].el);
    x_159 = x_158;
  }
  if (x_159) {
    float v_2 = float(x_6.x_GLF_uniform_int_values[2].el);
    float v_3 = float(x_6.x_GLF_uniform_int_values[1].el);
    float v_4 = float(x_6.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v_2, v_3, v_4, float(x_6.x_GLF_uniform_int_values[2].el));
  } else {
    x_GLF_color = vec4(float(x_6.x_GLF_uniform_int_values[1].el));
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
