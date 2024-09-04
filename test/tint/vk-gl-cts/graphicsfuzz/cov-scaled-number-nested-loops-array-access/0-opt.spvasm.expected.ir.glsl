SKIP: FAILED

#version 310 es

struct strided_arr {
  int el;
};

struct buf0 {
  strided_arr x_GLF_uniform_int_values[7];
};

struct strided_arr_1 {
  float el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_float_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_6;
uniform buf1 x_8;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float sums[2] = float[2](0.0f, 0.0f);
  int a = 0;
  int b = 0;
  int c = 0;
  int d = 0;
  int e = 0;
  int f = 0;
  int g = 0;
  int h = 0;
  int i = 0;
  int j = 0;
  bool x_215 = false;
  bool x_216 = false;
  int x_20 = x_6.x_GLF_uniform_int_values[1].el;
  sums[x_20] = x_8.x_GLF_uniform_float_values[0].el;
  int x_22 = x_6.x_GLF_uniform_int_values[2].el;
  sums[x_22] = x_8.x_GLF_uniform_float_values[0].el;
  a = x_6.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((a < x_6.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      b = x_6.x_GLF_uniform_int_values[5].el;
      {
        while(true) {
          if ((b < x_6.x_GLF_uniform_int_values[3].el)) {
          } else {
            break;
          }
          c = x_6.x_GLF_uniform_int_values[6].el;
          {
            while(true) {
              if ((c <= x_6.x_GLF_uniform_int_values[4].el)) {
              } else {
                break;
              }
              d = x_6.x_GLF_uniform_int_values[1].el;
              {
                while(true) {
                  if ((d < x_6.x_GLF_uniform_int_values[6].el)) {
                  } else {
                    break;
                  }
                  e = x_6.x_GLF_uniform_int_values[0].el;
                  {
                    while(true) {
                      if ((e <= x_6.x_GLF_uniform_int_values[4].el)) {
                      } else {
                        break;
                      }
                      f = x_6.x_GLF_uniform_int_values[1].el;
                      {
                        while(true) {
                          if ((f < x_6.x_GLF_uniform_int_values[0].el)) {
                          } else {
                            break;
                          }
                          g = x_6.x_GLF_uniform_int_values[1].el;
                          {
                            while(true) {
                              if ((g < x_6.x_GLF_uniform_int_values[6].el)) {
                              } else {
                                break;
                              }
                              h = x_6.x_GLF_uniform_int_values[1].el;
                              {
                                while(true) {
                                  if ((h < x_6.x_GLF_uniform_int_values[0].el)) {
                                  } else {
                                    break;
                                  }
                                  i = x_6.x_GLF_uniform_int_values[1].el;
                                  {
                                    while(true) {
                                      if ((i < x_6.x_GLF_uniform_int_values[4].el)) {
                                      } else {
                                        break;
                                      }
                                      j = x_6.x_GLF_uniform_int_values[0].el;
                                      {
                                        while(true) {
                                          if ((j > x_6.x_GLF_uniform_int_values[1].el)) {
                                          } else {
                                            break;
                                          }
                                          int x_53 = a;
                                          sums[x_53] = (sums[a] + x_8.x_GLF_uniform_float_values[2].el);
                                          {
                                            j = (j - 1);
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
                                    h = (h + 1);
                                  }
                                  continue;
                                }
                              }
                              {
                                g = (g + 1);
                              }
                              continue;
                            }
                          }
                          {
                            f = (f + 1);
                          }
                          continue;
                        }
                      }
                      {
                        e = (e + 1);
                      }
                      continue;
                    }
                  }
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
  bool x_207 = (sums[x_6.x_GLF_uniform_int_values[1].el] == x_8.x_GLF_uniform_float_values[1].el);
  x_216 = x_207;
  if (x_207) {
    x_215 = (sums[x_6.x_GLF_uniform_int_values[2].el] == x_8.x_GLF_uniform_float_values[1].el);
    x_216 = x_215;
  }
  if (x_216) {
    float v = float(x_6.x_GLF_uniform_int_values[2].el);
    float v_1 = float(x_6.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_6.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_6.x_GLF_uniform_int_values[2].el));
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
