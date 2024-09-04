SKIP: FAILED

#version 310 es

struct buf0 {
  int one;
};

struct main_out {
  vec4 x_GLF_color_1;
};
precision highp float;
precision highp int;


uniform buf0 x_7;
vec4 x_GLF_color = vec4(0.0f);
void main_1() {
  int a = 0;
  int i0 = 0;
  int i1 = 0;
  int i2 = 0;
  int i3 = 0;
  int i4 = 0;
  int i5 = 0;
  int i6 = 0;
  int i7 = 0;
  int i8_1 = 0;
  int i9 = 0;
  a = 0;
  i0 = 0;
  {
    while(true) {
      int x_40 = i0;
      int x_42 = x_7.one;
      if ((x_40 < x_42)) {
      } else {
        break;
      }
      i1 = 0;
      {
        while(true) {
          int x_49 = i1;
          int x_51 = x_7.one;
          if ((x_49 < x_51)) {
          } else {
            break;
          }
          i2 = 0;
          {
            while(true) {
              int x_58 = i2;
              int x_60 = x_7.one;
              if ((x_58 < x_60)) {
              } else {
                break;
              }
              i3 = 0;
              {
                while(true) {
                  int x_67 = i3;
                  int x_69 = x_7.one;
                  if ((x_67 < (x_69 + 2))) {
                  } else {
                    break;
                  }
                  i4 = 0;
                  {
                    while(true) {
                      int x_77 = i4;
                      int x_79 = x_7.one;
                      if ((x_77 < x_79)) {
                      } else {
                        break;
                      }
                      i5 = 0;
                      {
                        while(true) {
                          int x_86 = i5;
                          int x_88 = x_7.one;
                          if ((x_86 < x_88)) {
                          } else {
                            break;
                          }
                          {
                            while(true) {
                              int x_96 = x_7.one;
                              if ((x_96 > 0)) {
                              } else {
                                break;
                              }
                              i6 = 0;
                              {
                                while(true) {
                                  int x_103 = i6;
                                  int x_105 = x_7.one;
                                  if ((x_103 < x_105)) {
                                  } else {
                                    break;
                                  }
                                  i7 = 0;
                                  {
                                    while(true) {
                                      int x_112 = i7;
                                      int x_114 = x_7.one;
                                      if ((x_112 < x_114)) {
                                      } else {
                                        break;
                                      }
                                      i8_1 = 0;
                                      {
                                        while(true) {
                                          int x_121 = i8_1;
                                          int x_123 = x_7.one;
                                          if ((x_121 < x_123)) {
                                          } else {
                                            break;
                                          }
                                          i9 = 0;
                                          {
                                            while(true) {
                                              int x_130 = i9;
                                              int x_132 = x_7.one;
                                              if ((x_130 < x_132)) {
                                              } else {
                                                break;
                                              }
                                              int x_135 = a;
                                              a = (x_135 + 1);
                                              {
                                                int x_137 = i9;
                                                i9 = (x_137 + 1);
                                              }
                                              continue;
                                            }
                                          }
                                          {
                                            int x_139 = i8_1;
                                            i8_1 = (x_139 + 1);
                                          }
                                          continue;
                                        }
                                      }
                                      {
                                        int x_141 = i7;
                                        i7 = (x_141 + 1);
                                      }
                                      continue;
                                    }
                                  }
                                  {
                                    int x_143 = i6;
                                    i6 = (x_143 + 1);
                                  }
                                  continue;
                                }
                              }
                              break;
                            }
                          }
                          {
                            int x_145 = i5;
                            i5 = (x_145 + 1);
                          }
                          continue;
                        }
                      }
                      {
                        int x_147 = i4;
                        i4 = (x_147 + 1);
                      }
                      continue;
                    }
                  }
                  {
                    int x_149 = i3;
                    i3 = (x_149 + 1);
                  }
                  continue;
                }
              }
              {
                int x_151 = i2;
                i2 = (x_151 + 1);
              }
              continue;
            }
          }
          {
            int x_153 = i1;
            i1 = (x_153 + 1);
          }
          continue;
        }
      }
      {
        int x_155 = i0;
        i0 = (x_155 + 1);
      }
      continue;
    }
  }
  int x_157 = a;
  if ((x_157 == 3)) {
    x_GLF_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4(0.0f);
  }
}
main_out main() {
  main_1();
  return main_out(x_GLF_color);
}
error: Error parsing GLSL shader:
ERROR: 0:8: 'float' : type requires declaration of default precision qualifier 
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.




tint executable returned error: exit status 1
