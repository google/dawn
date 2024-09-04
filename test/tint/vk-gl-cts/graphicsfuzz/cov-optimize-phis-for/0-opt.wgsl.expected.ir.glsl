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
  float x_104 = x_7.x_GLF_uniform_float_values[0].el;
  a = x_104;
  float x_106 = x_7.x_GLF_uniform_float_values[1].el;
  b = x_106;
  int x_24 = x_11.x_GLF_uniform_int_values[1].el;
  i = x_24;
  {
    while(true) {
      int x_25 = i;
      int x_26 = x_11.x_GLF_uniform_int_values[0].el;
      if ((x_25 < x_26)) {
      } else {
        break;
      }
      int x_27 = x_11.x_GLF_uniform_int_values[1].el;
      i_1 = x_27;
      {
        while(true) {
          int x_28 = i_1;
          int x_29 = x_11.x_GLF_uniform_int_values[0].el;
          if ((x_28 < x_29)) {
          } else {
            break;
          }
          int x_30 = x_11.x_GLF_uniform_int_values[1].el;
          i_2 = x_30;
          {
            while(true) {
              int x_31 = i_2;
              int x_32 = x_11.x_GLF_uniform_int_values[0].el;
              if ((x_31 < x_32)) {
              } else {
                break;
              }
              int x_33 = x_11.x_GLF_uniform_int_values[2].el;
              i_3 = x_33;
              {
                while(true) {
                  int x_34 = i_3;
                  int x_35 = x_11.x_GLF_uniform_int_values[0].el;
                  if ((x_34 < x_35)) {
                  } else {
                    break;
                  }
                  int x_36 = x_11.x_GLF_uniform_int_values[2].el;
                  i_4 = x_36;
                  {
                    while(true) {
                      int x_37 = i_4;
                      int x_38 = x_11.x_GLF_uniform_int_values[0].el;
                      if ((x_37 < x_38)) {
                      } else {
                        break;
                      }
                      int x_39 = x_11.x_GLF_uniform_int_values[1].el;
                      i_5 = x_39;
                      {
                        while(true) {
                          int x_40 = i_5;
                          int x_41 = x_11.x_GLF_uniform_int_values[0].el;
                          if ((x_40 < x_41)) {
                          } else {
                            break;
                          }
                          int x_42 = x_11.x_GLF_uniform_int_values[1].el;
                          i_6 = x_42;
                          {
                            while(true) {
                              int x_43 = i_6;
                              int x_44 = x_11.x_GLF_uniform_int_values[0].el;
                              if ((x_43 < x_44)) {
                              } else {
                                break;
                              }
                              int x_45 = x_11.x_GLF_uniform_int_values[1].el;
                              i_7 = x_45;
                              {
                                while(true) {
                                  int x_46 = i_7;
                                  int x_47 = x_11.x_GLF_uniform_int_values[0].el;
                                  if ((x_46 < x_47)) {
                                  } else {
                                    break;
                                  }
                                  int x_48 = x_11.x_GLF_uniform_int_values[1].el;
                                  i_8 = x_48;
                                  {
                                    while(true) {
                                      int x_49 = i_8;
                                      int x_50 = x_11.x_GLF_uniform_int_values[0].el;
                                      if ((x_49 < x_50)) {
                                      } else {
                                        break;
                                      }
                                      int x_51 = x_11.x_GLF_uniform_int_values[1].el;
                                      i_9 = x_51;
                                      {
                                        while(true) {
                                          int x_52 = i_9;
                                          int x_53 = x_11.x_GLF_uniform_int_values[0].el;
                                          if ((x_52 < x_53)) {
                                          } else {
                                            break;
                                          }
                                          int x_54 = x_11.x_GLF_uniform_int_values[1].el;
                                          i_10 = x_54;
                                          {
                                            while(true) {
                                              int x_55 = i_10;
                                              int x_56 = x_11.x_GLF_uniform_int_values[0].el;
                                              if ((x_55 < x_56)) {
                                              } else {
                                                break;
                                              }
                                              float x_196 = x_7.x_GLF_uniform_float_values[1].el;
                                              a = x_196;
                                              float x_198 = tint_symbol.y;
                                              float x_200 = x_7.x_GLF_uniform_float_values[1].el;
                                              if ((x_198 > x_200)) {
                                                break;
                                              }
                                              {
                                                int x_57 = i_10;
                                                i_10 = (x_57 + 1);
                                              }
                                              continue;
                                            }
                                          }
                                          {
                                            int x_59 = i_9;
                                            i_9 = (x_59 + 1);
                                          }
                                          continue;
                                        }
                                      }
                                      {
                                        int x_61 = i_8;
                                        i_8 = (x_61 + 1);
                                      }
                                      continue;
                                    }
                                  }
                                  {
                                    int x_63 = i_7;
                                    i_7 = (x_63 + 1);
                                  }
                                  continue;
                                }
                              }
                              {
                                int x_65 = i_6;
                                i_6 = (x_65 + 1);
                              }
                              continue;
                            }
                          }
                          {
                            int x_67 = i_5;
                            i_5 = (x_67 + 1);
                          }
                          continue;
                        }
                      }
                      {
                        int x_69 = i_4;
                        i_4 = (x_69 + 1);
                      }
                      continue;
                    }
                  }
                  {
                    int x_71 = i_3;
                    i_3 = (x_71 + 1);
                  }
                  continue;
                }
              }
              {
                int x_73 = i_2;
                i_2 = (x_73 + 1);
              }
              continue;
            }
          }
          {
            int x_75 = i_1;
            i_1 = (x_75 + 1);
          }
          continue;
        }
      }
      float x_204 = b;
      b = (x_204 + 1.0f);
      {
        int x_77 = i;
        i = (x_77 + 1);
      }
      continue;
    }
  }
  float x_206 = b;
  float x_207 = a;
  float x_208 = a;
  float x_209 = b;
  x_GLF_color = vec4(x_206, x_207, x_208, x_209);
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
