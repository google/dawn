SKIP: FAILED

#version 310 es

struct strided_arr {
  float el;
};

struct buf0 {
  strided_arr x_GLF_uniform_float_values[3];
};

struct strided_arr_1 {
  int el;
};

struct buf1 {
  strided_arr_1 x_GLF_uniform_int_values[4];
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


int x_GLF_global_loop_count = 0;
uniform buf0 x_7;
uniform buf1 x_10;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  float f = 0.0f;
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
  int i_11 = 0;
  int i_12 = 0;
  int i_13 = 0;
  int i_14 = 0;
  float sum = 0.0f;
  int r = 0;
  x_GLF_global_loop_count = 0;
  f = x_7.x_GLF_uniform_float_values[1].el;
  i = x_10.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((i < x_10.x_GLF_uniform_int_values[0].el)) {
      } else {
        break;
      }
      i_1 = x_10.x_GLF_uniform_int_values[1].el;
      {
        while(true) {
          if ((i_1 < x_10.x_GLF_uniform_int_values[0].el)) {
          } else {
            break;
          }
          i_2 = x_10.x_GLF_uniform_int_values[1].el;
          {
            while(true) {
              if ((i_2 < x_10.x_GLF_uniform_int_values[0].el)) {
              } else {
                break;
              }
              i_3 = x_10.x_GLF_uniform_int_values[1].el;
              {
                while(true) {
                  if ((i_3 < x_10.x_GLF_uniform_int_values[0].el)) {
                  } else {
                    break;
                  }
                  i_4 = x_10.x_GLF_uniform_int_values[1].el;
                  {
                    while(true) {
                      if ((i_4 < x_10.x_GLF_uniform_int_values[0].el)) {
                      } else {
                        break;
                      }
                      i_5 = x_10.x_GLF_uniform_int_values[1].el;
                      {
                        while(true) {
                          if ((i_5 < x_10.x_GLF_uniform_int_values[0].el)) {
                          } else {
                            break;
                          }
                          i_6 = x_10.x_GLF_uniform_int_values[1].el;
                          {
                            while(true) {
                              if ((i_6 < x_10.x_GLF_uniform_int_values[0].el)) {
                              } else {
                                break;
                              }
                              i_7 = x_10.x_GLF_uniform_int_values[1].el;
                              {
                                while(true) {
                                  if ((i_7 < x_10.x_GLF_uniform_int_values[0].el)) {
                                  } else {
                                    break;
                                  }
                                  i_8 = x_10.x_GLF_uniform_int_values[1].el;
                                  {
                                    while(true) {
                                      if ((i_8 < x_10.x_GLF_uniform_int_values[0].el)) {
                                      } else {
                                        break;
                                      }
                                      i_9 = x_10.x_GLF_uniform_int_values[1].el;
                                      {
                                        while(true) {
                                          if ((i_9 < x_10.x_GLF_uniform_int_values[0].el)) {
                                          } else {
                                            break;
                                          }
                                          i_10 = x_10.x_GLF_uniform_int_values[1].el;
                                          {
                                            while(true) {
                                              if ((i_10 < x_10.x_GLF_uniform_int_values[0].el)) {
                                              } else {
                                                break;
                                              }
                                              i_11 = x_10.x_GLF_uniform_int_values[1].el;
                                              {
                                                while(true) {
                                                  if ((i_11 < x_10.x_GLF_uniform_int_values[2].el)) {
                                                  } else {
                                                    break;
                                                  }
                                                  i_12 = x_10.x_GLF_uniform_int_values[1].el;
                                                  {
                                                    while(true) {
                                                      if ((i_12 < x_10.x_GLF_uniform_int_values[0].el)) {
                                                      } else {
                                                        break;
                                                      }
                                                      i_13 = x_10.x_GLF_uniform_int_values[1].el;
                                                      {
                                                        while(true) {
                                                          if ((i_13 < x_10.x_GLF_uniform_int_values[0].el)) {
                                                          } else {
                                                            break;
                                                          }
                                                          i_14 = x_10.x_GLF_uniform_int_values[1].el;
                                                          {
                                                            while(true) {
                                                              if ((i_14 < x_10.x_GLF_uniform_int_values[2].el)) {
                                                              } else {
                                                                break;
                                                              }
                                                              {
                                                                while(true) {
                                                                  x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
                                                                  {
                                                                    int x_225 = x_GLF_global_loop_count;
                                                                    int x_227 = x_10.x_GLF_uniform_int_values[3].el;
                                                                    if (!((x_225 < (100 - x_227)))) { break; }
                                                                  }
                                                                  continue;
                                                                }
                                                              }
                                                              f = (f + x_7.x_GLF_uniform_float_values[0].el);
                                                              {
                                                                i_14 = (i_14 + 1);
                                                              }
                                                              continue;
                                                            }
                                                          }
                                                          {
                                                            i_13 = (i_13 + 1);
                                                          }
                                                          continue;
                                                        }
                                                      }
                                                      {
                                                        i_12 = (i_12 + 1);
                                                      }
                                                      continue;
                                                    }
                                                  }
                                                  {
                                                    i_11 = (i_11 + 1);
                                                  }
                                                  continue;
                                                }
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
      {
        i = (i + 1);
      }
      continue;
    }
  }
  sum = x_7.x_GLF_uniform_float_values[1].el;
  r = x_10.x_GLF_uniform_int_values[1].el;
  {
    while(true) {
      if ((x_GLF_global_loop_count < 100)) {
      } else {
        break;
      }
      x_GLF_global_loop_count = (x_GLF_global_loop_count + 1);
      sum = (sum + f);
      {
        r = (r + 1);
      }
      continue;
    }
  }
  if ((sum == x_7.x_GLF_uniform_float_values[2].el)) {
    float v = float(x_10.x_GLF_uniform_int_values[0].el);
    float v_1 = float(x_10.x_GLF_uniform_int_values[1].el);
    float v_2 = float(x_10.x_GLF_uniform_int_values[1].el);
    x_GLF_color = vec4(v, v_1, v_2, float(x_10.x_GLF_uniform_int_values[0].el));
  } else {
    x_GLF_color = vec4(float(x_10.x_GLF_uniform_int_values[1].el));
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
