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
      if ((i0 < x_7.one)) {
      } else {
        break;
      }
      i1 = 0;
      {
        while(true) {
          if ((i1 < x_7.one)) {
          } else {
            break;
          }
          i2 = 0;
          {
            while(true) {
              if ((i2 < x_7.one)) {
              } else {
                break;
              }
              i3 = 0;
              {
                while(true) {
                  if ((i3 < (x_7.one + 2))) {
                  } else {
                    break;
                  }
                  i4 = 0;
                  {
                    while(true) {
                      if ((i4 < x_7.one)) {
                      } else {
                        break;
                      }
                      i5 = 0;
                      {
                        while(true) {
                          if ((i5 < x_7.one)) {
                          } else {
                            break;
                          }
                          {
                            while(true) {
                              if ((x_7.one > 0)) {
                              } else {
                                break;
                              }
                              i6 = 0;
                              {
                                while(true) {
                                  if ((i6 < x_7.one)) {
                                  } else {
                                    break;
                                  }
                                  i7 = 0;
                                  {
                                    while(true) {
                                      if ((i7 < x_7.one)) {
                                      } else {
                                        break;
                                      }
                                      i8_1 = 0;
                                      {
                                        while(true) {
                                          if ((i8_1 < x_7.one)) {
                                          } else {
                                            break;
                                          }
                                          i9 = 0;
                                          {
                                            while(true) {
                                              if ((i9 < x_7.one)) {
                                              } else {
                                                break;
                                              }
                                              a = (a + 1);
                                              {
                                                i9 = (i9 + 1);
                                              }
                                              continue;
                                            }
                                          }
                                          {
                                            i8_1 = (i8_1 + 1);
                                          }
                                          continue;
                                        }
                                      }
                                      {
                                        i7 = (i7 + 1);
                                      }
                                      continue;
                                    }
                                  }
                                  {
                                    i6 = (i6 + 1);
                                  }
                                  continue;
                                }
                              }
                              break;
                            }
                          }
                          {
                            i5 = (i5 + 1);
                          }
                          continue;
                        }
                      }
                      {
                        i4 = (i4 + 1);
                      }
                      continue;
                    }
                  }
                  {
                    i3 = (i3 + 1);
                  }
                  continue;
                }
              }
              {
                i2 = (i2 + 1);
              }
              continue;
            }
          }
          {
            i1 = (i1 + 1);
          }
          continue;
        }
      }
      {
        i0 = (i0 + 1);
      }
      continue;
    }
  }
  if ((a == 3)) {
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
