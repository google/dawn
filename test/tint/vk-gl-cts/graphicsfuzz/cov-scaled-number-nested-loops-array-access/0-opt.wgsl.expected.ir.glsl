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
  bool x_216_phi = false;
  int x_20 = x_6.x_GLF_uniform_int_values[1].el;
  float x_110 = x_8.x_GLF_uniform_float_values[0].el;
  sums[x_20] = x_110;
  int x_22 = x_6.x_GLF_uniform_int_values[2].el;
  float x_114 = x_8.x_GLF_uniform_float_values[0].el;
  sums[x_22] = x_114;
  int x_23 = x_6.x_GLF_uniform_int_values[1].el;
  a = x_23;
  {
    while(true) {
      int x_24 = a;
      int x_25 = x_6.x_GLF_uniform_int_values[0].el;
      if ((x_24 < x_25)) {
      } else {
        break;
      }
      int x_26 = x_6.x_GLF_uniform_int_values[5].el;
      b = x_26;
      {
        while(true) {
          int x_27 = b;
          int x_28 = x_6.x_GLF_uniform_int_values[3].el;
          if ((x_27 < x_28)) {
          } else {
            break;
          }
          int x_29 = x_6.x_GLF_uniform_int_values[6].el;
          c = x_29;
          {
            while(true) {
              int x_30 = c;
              int x_31 = x_6.x_GLF_uniform_int_values[4].el;
              if ((x_30 <= x_31)) {
              } else {
                break;
              }
              int x_32 = x_6.x_GLF_uniform_int_values[1].el;
              d = x_32;
              {
                while(true) {
                  int x_33 = d;
                  int x_34 = x_6.x_GLF_uniform_int_values[6].el;
                  if ((x_33 < x_34)) {
                  } else {
                    break;
                  }
                  int x_35 = x_6.x_GLF_uniform_int_values[0].el;
                  e = x_35;
                  {
                    while(true) {
                      int x_36 = e;
                      int x_37 = x_6.x_GLF_uniform_int_values[4].el;
                      if ((x_36 <= x_37)) {
                      } else {
                        break;
                      }
                      int x_38 = x_6.x_GLF_uniform_int_values[1].el;
                      f = x_38;
                      {
                        while(true) {
                          int x_39 = f;
                          int x_40 = x_6.x_GLF_uniform_int_values[0].el;
                          if ((x_39 < x_40)) {
                          } else {
                            break;
                          }
                          int x_41 = x_6.x_GLF_uniform_int_values[1].el;
                          g = x_41;
                          {
                            while(true) {
                              int x_42 = g;
                              int x_43 = x_6.x_GLF_uniform_int_values[6].el;
                              if ((x_42 < x_43)) {
                              } else {
                                break;
                              }
                              int x_44 = x_6.x_GLF_uniform_int_values[1].el;
                              h = x_44;
                              {
                                while(true) {
                                  int x_45 = h;
                                  int x_46 = x_6.x_GLF_uniform_int_values[0].el;
                                  if ((x_45 < x_46)) {
                                  } else {
                                    break;
                                  }
                                  int x_47 = x_6.x_GLF_uniform_int_values[1].el;
                                  i = x_47;
                                  {
                                    while(true) {
                                      int x_48 = i;
                                      int x_49 = x_6.x_GLF_uniform_int_values[4].el;
                                      if ((x_48 < x_49)) {
                                      } else {
                                        break;
                                      }
                                      int x_50 = x_6.x_GLF_uniform_int_values[0].el;
                                      j = x_50;
                                      {
                                        while(true) {
                                          int x_51 = j;
                                          int x_52 = x_6.x_GLF_uniform_int_values[1].el;
                                          if ((x_51 > x_52)) {
                                          } else {
                                            break;
                                          }
                                          int x_53 = a;
                                          float x_197 = x_8.x_GLF_uniform_float_values[2].el;
                                          float x_199 = sums[x_53];
                                          sums[x_53] = (x_199 + x_197);
                                          {
                                            int x_54 = j;
                                            j = (x_54 - 1);
                                          }
                                          continue;
                                        }
                                      }
                                      {
                                        int x_56 = i;
                                        i = (x_56 + 1);
                                      }
                                      continue;
                                    }
                                  }
                                  {
                                    int x_58 = h;
                                    h = (x_58 + 1);
                                  }
                                  continue;
                                }
                              }
                              {
                                int x_60 = g;
                                g = (x_60 + 1);
                              }
                              continue;
                            }
                          }
                          {
                            int x_62 = f;
                            f = (x_62 + 1);
                          }
                          continue;
                        }
                      }
                      {
                        int x_64 = e;
                        e = (x_64 + 1);
                      }
                      continue;
                    }
                  }
                  {
                    int x_66 = d;
                    d = (x_66 + 1);
                  }
                  continue;
                }
              }
              {
                int x_68 = c;
                c = (x_68 + 1);
              }
              continue;
            }
          }
          {
            int x_70 = b;
            b = (x_70 + 1);
          }
          continue;
        }
      }
      {
        int x_72 = a;
        a = (x_72 + 1);
      }
      continue;
    }
  }
  int x_74 = x_6.x_GLF_uniform_int_values[1].el;
  float x_204 = sums[x_74];
  float x_206 = x_8.x_GLF_uniform_float_values[1].el;
  bool x_207 = (x_204 == x_206);
  x_216_phi = x_207;
  if (x_207) {
    int x_75 = x_6.x_GLF_uniform_int_values[2].el;
    float x_212 = sums[x_75];
    float x_214 = x_8.x_GLF_uniform_float_values[1].el;
    x_215 = (x_212 == x_214);
    x_216_phi = x_215;
  }
  bool x_216 = x_216_phi;
  if (x_216) {
    int x_76 = x_6.x_GLF_uniform_int_values[2].el;
    int x_77 = x_6.x_GLF_uniform_int_values[1].el;
    int x_78 = x_6.x_GLF_uniform_int_values[1].el;
    int x_79 = x_6.x_GLF_uniform_int_values[2].el;
    float v = float(x_76);
    float v_1 = float(x_77);
    float v_2 = float(x_78);
    x_GLF_color = vec4(v, v_1, v_2, float(x_79));
  } else {
    int x_80 = x_6.x_GLF_uniform_int_values[1].el;
    float x_230 = float(x_80);
    x_GLF_color = vec4(x_230, x_230, x_230, x_230);
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
