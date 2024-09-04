SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[2];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[3];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_7;
uniform buf1 x_11;
vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  int i = 0;
  int i_1 = 0;
  int i_2 = 0;
  int i_3 = 0;
  int i_4 = 0;
  int i_5 = 0;
  int i_6 = 0;
  int i_7 = 0;
  int i_8 = 0;
  int i_9 = 0;
  int i_10 = 0;
  a = x_7.x_GLF_uniform_float_values[0].el;
  b = x_7.x_GLF_uniform_float_values[1].el;
  i = x_11.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_11.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      i_1 = x_11.x_GLF_uniform_int_values[1].el;
      {
        while(true) {
          if ((i_1 < x_11.x_GLF_uniform_int_values[0].el)) {
          } else {
            break;
          }
          i_2 = x_11.x_GLF_uniform_int_values[1].el;
          {
            while(true) {
              if ((i_2 < x_11.x_GLF_uniform_int_values[0].el)) {
              } else {
                break;
              }
              i_3 = x_11.x_GLF_uniform_int_values[2].el;
              {
                while(true) {
                  if ((i_3 < x_11.x_GLF_uniform_int_values[0].el)) {
                  } else {
                    break;
                  }
                  i_4 = x_11.x_GLF_uniform_int_values[2].el;
                  {
                    while(true) {
                      if ((i_4 < x_11.x_GLF_uniform_int_values[0].el)) {
                      } else {
                        break;
                      }
                      i_5 = x_11.x_GLF_uniform_int_values[1].el;
                      {
                        while(true) {
                          if ((i_5 < x_11.x_GLF_uniform_int_values[0].el)) {
                          } else {
                            break;
                          }
                          i_6 = x_11.x_GLF_uniform_int_values[1].el;
                          {
                            while(true) {
                              if ((i_6 < x_11.x_GLF_uniform_int_values[0].el)) {
                              } else {
                                break;
                              }
                              i_7 = x_11.x_GLF_uniform_int_values[1].el;
                              {
                                while(true) {
                                  if ((i_7 < x_11.x_GLF_uniform_int_values[0].el)) {
                                  } else {
                                    break;
                                  }
                                  i_8 = x_11.x_GLF_uniform_int_values[1].el;
                                  {
                                    while(true) {
                                      if ((i_8 < x_11.x_GLF_uniform_int_values[0].el)) {
                                      } else {
                                        break;
                                      }
                                      i_9 = x_11.x_GLF_uniform_int_values[1].el;
                                      {
                                        while(true) {
                                          if ((i_9 < x_11.x_GLF_uniform_int_values[0].el)) {
                                          } else {
                                            break;
                                          }
                                          i_10 = x_11.x_GLF_uniform_int_values[1].el;
                                          {
                                            while(true) {
                                              if ((i_10 < x_11.x_GLF_uniform_int_values[0].el)) {
                                              } else {
                                                break;
                                              }
                                              a = x_7.x_GLF_uniform_float_values[1].el;
                                              if ((tint_symbol.y > x_7.x_GLF_uniform_float_values[1].el)) {
                                                break;
                                              }
                                              {
                                                i_10 = (i_10 + 1);
                                              }
                                              continue;
                                            }
                                          }
                                          {
                                            i_9 = (i_9 + 1);
                                          }
                                          continue;
                                        }
                                      }
                                      {
                                        i_8 = (i_8 + 1);
                                      }
                                      continue;
                                    }
                                  }
                                  {
                                    i_7 = (i_7 + 1);
                                  }
                                  continue;
                                }
                              }
                              {
                                i_6 = (i_6 + 1);
                              }
                              continue;
                            }
                          }
                          {
                            i_5 = (i_5 + 1);
                          }
                          continue;
                        }
                      }
                      {
                        i_4 = (i_4 + 1);
                      }
                      continue;
                    }
                  }
                  {
                    i_3 = (i_3 + 1);
                  }
                  continue;
                }
              }
              {
                i_2 = (i_2 + 1);
              }
              continue;
            }
          }
          {
            i_1 = (i_1 + 1);
          }
          continue;
        }
      }
      b = (b + 1.0f);
      {
        i = (i + 1);
      }
      continue;
    }
  }
  x_GLF_color = vec4(b, a, a, b);
}
main_out main(vec4 tint_symbol_2) {
  tint_symbol = tint_symbol_2;
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:4: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
