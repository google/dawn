SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};

struct main_inputs {
  float4 gl_FragCoord_param : SV_Position;
};


static float4 gl_FragCoord = (0.0f).xxxx;
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float4 c = (0.0f).xxxx;
  int a = 0;
  int i1 = 0;
  int i2 = 0;
  int i3 = 0;
  int i4 = 0;
  int i5 = 0;
  int i6 = 0;
  int i7 = 0;
  int i8_1 = 0;
  c = float4(0.0f, 0.0f, 0.0f, 1.0f);
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
            float x_123 = gl_FragCoord.x;
            if (!((x_123 < -1.0f))) { break; }
          }
          continue;
        }
      }
      {
        float x_126 = gl_FragCoord.y;
        if (!((x_126 < -1.0f))) { break; }
      }
      continue;
    }
  }
  x_GLF_color = c;
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v = {x_GLF_color};
  return v;
}

main_outputs main(main_inputs inputs) {
  main_out v_1 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_2 = {v_1.x_GLF_color_1};
  return v_2;
}

FXC validation failure:
<scrubbed_path>(32,9-19): error X3511: forced to unroll loop, but unrolling failed.
<scrubbed_path>(30,5-15): error X3511: forced to unroll loop, but unrolling failed.


tint executable returned error: exit status 1
