SKIP: FAILED

#version 310 es

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


vec4 tint_symbol = vec4(0.0f);
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  vec4 c = vec4(0.0f);
  int a = 0;
  int i1 = 0;
  int i2 = 0;
  int i3 = 0;
  int i4 = 0;
  int i5 = 0;
  int i6 = 0;
  int i7 = 0;
  int i8_1 = 0;
  c = vec4(0.0f, 0.0f, 0.0f, 1.0f);
  a = 0;
  {
    while(true) {
      {
        while(true) {
          int x_46 = a;
          c[x_46] = 1.0f;
          i1 = 0;
          {
            while(true) {
              int x_52 = i1;
              if ((x_52 < 1)) {
              } else {
                break;
              }
              i2 = 0;
              {
                while(true) {
                  int x_59 = i2;
                  if ((x_59 < 1)) {
                  } else {
                    break;
                  }
                  i3 = 0;
                  {
                    while(true) {
                      int x_66 = i3;
                      if ((x_66 < 1)) {
                      } else {
                        break;
                      }
                      i4 = 0;
                      {
                        while(true) {
                          int x_73 = i4;
                          if ((x_73 < 1)) {
                          } else {
                            break;
                          }
                          i5 = 0;
                          {
                            while(true) {
                              int x_80 = i5;
                              if ((x_80 < 1)) {
                              } else {
                                break;
                              }
                              i6 = 0;
                              {
                                while(true) {
                                  int x_87 = i6;
                                  if ((x_87 < 1)) {
                                  } else {
                                    break;
                                  }
                                  i7 = 0;
                                  {
                                    while(true) {
                                      int x_94 = i7;
                                      if ((x_94 < 1)) {
                                      } else {
                                        break;
                                      }
                                      i8_1 = 0;
                                      {
                                        while(true) {
                                          int x_101 = i8_1;
                                          if ((x_101 < 17)) {
                                          } else {
                                            break;
                                          }
                                          int x_104 = a;
                                          a = (x_104 + 1);
                                          {
                                            int x_106 = i8_1;
                                            i8_1 = (x_106 + 1);
                                          }
                                          continue;
                                        }
                                      }
                                      {
                                        int x_108 = i7;
                                        i7 = (x_108 + 1);
                                      }
                                      continue;
                                    }
                                  }
                                  {
                                    int x_110 = i6;
                                    i6 = (x_110 + 1);
                                  }
                                  continue;
                                }
                              }
                              {
                                int x_112 = i5;
                                i5 = (x_112 + 1);
                              }
                              continue;
                            }
                          }
                          {
                            int x_114 = i4;
                            i4 = (x_114 + 1);
                          }
                          continue;
                        }
                      }
                      {
                        int x_116 = i3;
                        i3 = (x_116 + 1);
                      }
                      continue;
                    }
                  }
                  {
                    int x_118 = i2;
                    i2 = (x_118 + 1);
                  }
                  continue;
                }
              }
              {
                int x_120 = i1;
                i1 = (x_120 + 1);
              }
              continue;
            }
          }
          {
            float x_123 = tint_symbol.x;
            if (!((x_123 < -1.0f))) { break; }
          }
          continue;
        }
      }
      {
        float x_126 = tint_symbol.y;
        if (!((x_126 < -1.0f))) { break; }
      }
      continue;
    }
  }
  vec4 x_128 = c;
  x_GLF_color = x_128;
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
