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
  float4 x_128 = c;
  x_GLF_color = x_128;
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
