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
  bool x_102 = false;
  A[0] = x_6.x_GLF_uniform_float_values[1].el;
  A[1] = x_6.x_GLF_uniform_float_values[1].el;
  i = x_9.x_GLF_uniform_int_values[0].el;
  {
    while(true) {
      if ((i < x_9.x_GLF_uniform_int_values[3].el)) {
      } else {
        break;
      }
      j = x_9.x_GLF_uniform_int_values[0].el;
      {
        while(true) {
          if ((j < x_9.x_GLF_uniform_int_values[2].el)) {
          } else {
            break;
          }
          int x_66 = j;
          switch(x_66) {
            case 1:
            {
              int x_78 = i;
              A[x_78] = x_6.x_GLF_uniform_float_values[0].el;
              break;
            }
            case 0:
            {
              if (((-2147483647 - 1) < i)) {
                {
                  j = (j + 1);
                }
                continue;
              }
              int x_74 = i;
              A[x_74] = x_6.x_GLF_uniform_float_values[2].el;
              break;
            }
            default:
            {
              break;
            }
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
  bool x_92 = (A[x_9.x_GLF_uniform_int_values[0].el] == x_6.x_GLF_uniform_float_values[0].el);
  x_102 = x_92;
  if (x_92) {
    x_101 = (A[x_9.x_GLF_uniform_int_values[1].el] == x_6.x_GLF_uniform_float_values[0].el);
    x_102 = x_101;
  }
  if (x_102) {
    float v = float(x_9.x_GLF_uniform_int_values[1].el);
    float v_1 = float(x_9.x_GLF_uniform_int_values[0].el);
    float v_2 = float(x_9.x_GLF_uniform_int_values[0].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_9.x_GLF_uniform_int_values[1].el));
  } else {
    x_GLF_color = vec4(float(x_9.x_GLF_uniform_int_values[1].el));
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
