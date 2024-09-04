SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf1 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf0 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf1 x_6;
uniform buf0 x_9;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float A[2] = float[2](0.0f, 0.0f);
  int i = 0;
  int j = 0;
  bool x_101 = false;
  bool x_102_phi = false;
  float x_39 = x_6.x_GLF_uniform_float_values[1].el;
  A[0] = x_39;
  float x_42 = x_6.x_GLF_uniform_float_values[1].el;
  A[1] = x_42;
  int x_45 = x_9.x_GLF_uniform_int_values[0].el;
  i = x_45;
  {
    while(true) {
      int x_50 = i;
      int x_52 = x_9.x_GLF_uniform_int_values[3].el;
      if ((x_50 < x_52)) {
      } else {
        break;
      }
      int x_56 = x_9.x_GLF_uniform_int_values[0].el;
      j = x_56;
      {
        while(true) {
          int x_61 = j;
          int x_63 = x_9.x_GLF_uniform_int_values[2].el;
          if ((x_61 < x_63)) {
          } else {
            break;
          }
          int x_66 = j;
          switch(x_66) {
            case 1:
            {
              int x_78 = i;
              float x_80 = x_6.x_GLF_uniform_float_values[0].el;
              A[x_78] = x_80;
              break;
            }
            case 0:
            {
              int x_70 = i;
              if (((-2147483647 - 1) < x_70)) {
                {
                  int x_82 = j;
                  j = (x_82 + 1);
                }
                continue;
              }
              int x_74 = i;
              float x_76 = x_6.x_GLF_uniform_float_values[2].el;
              A[x_74] = x_76;
              break;
            }
            default:
            {
              break;
            }
          }
          {
            int x_82 = j;
            j = (x_82 + 1);
          }
          continue;
        }
      }
      {
        int x_84 = i;
        i = (x_84 + 1);
      }
      continue;
    }
  }
  int x_87 = x_9.x_GLF_uniform_int_values[0].el;
  float x_89 = A[x_87];
  float x_91 = x_6.x_GLF_uniform_float_values[0].el;
  bool x_92 = (x_89 == x_91);
  x_102_phi = x_92;
  if (x_92) {
    int x_96 = x_9.x_GLF_uniform_int_values[1].el;
    float x_98 = A[x_96];
    float x_100 = x_6.x_GLF_uniform_float_values[0].el;
    x_101 = (x_98 == x_100);
    x_102_phi = x_101;
  }
  bool x_102 = x_102_phi;
  if (x_102) {
    int x_107 = x_9.x_GLF_uniform_int_values[1].el;
    int x_110 = x_9.x_GLF_uniform_int_values[0].el;
    int x_113 = x_9.x_GLF_uniform_int_values[0].el;
    int x_116 = x_9.x_GLF_uniform_int_values[1].el;
    float v = float(x_107);
    float v_1 = float(x_110);
    float v_2 = float(x_113);
    x_GLF_color = vec4(v, v_1, v_2, float(x_116));
  } else {
    int x_120 = x_9.x_GLF_uniform_int_values[1].el;
    float x_121 = float(x_120);
    x_GLF_color = vec4(x_121, x_121, x_121, x_121);
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
