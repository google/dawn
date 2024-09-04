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
              if ((i1 < 1)) {
              } else {
                break;
              }
              i2 = 0;
              {
                while(true) {
                  if ((i2 < 1)) {
                  } else {
                    break;
                  }
                  i3 = 0;
                  {
                    while(true) {
                      if ((i3 < 1)) {
                      } else {
                        break;
                      }
                      i4 = 0;
                      {
                        while(true) {
                          if ((i4 < 1)) {
                          } else {
                            break;
                          }
                          i5 = 0;
                          {
                            while(true) {
                              if ((i5 < 1)) {
                              } else {
                                break;
                              }
                              i6 = 0;
                              {
                                while(true) {
                                  if ((i6 < 1)) {
                                  } else {
                                    break;
                                  }
                                  i7 = 0;
                                  {
                                    while(true) {
                                      if ((i7 < 1)) {
                                      } else {
                                        break;
                                      }
                                      i8_1 = 0;
                                      {
                                        while(true) {
                                          if ((i8_1 < 17)) {
                                          } else {
                                            break;
                                          }
                                          a = (a + 1);
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
  x_GLF_color = c;
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
