void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 c = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
  while (true) {
    while (true) {
      set_float4(c, a, 1.0f);
      i1 = 0;
      {
        for(; (i1 < 1); i1 = (i1 + 1)) {
          i2 = 0;
          {
            for(; (i2 < 1); i2 = (i2 + 1)) {
              i3 = 0;
              {
                for(; (i3 < 1); i3 = (i3 + 1)) {
                  i4 = 0;
                  {
                    for(; (i4 < 1); i4 = (i4 + 1)) {
                      i5 = 0;
                      {
                        for(; (i5 < 1); i5 = (i5 + 1)) {
                          i6 = 0;
                          {
                            for(; (i6 < 1); i6 = (i6 + 1)) {
                              i7 = 0;
                              {
                                for(; (i7 < 1); i7 = (i7 + 1)) {
                                  i8_1 = 0;
                                  {
                                    for(; (i8_1 < 17); i8_1 = (i8_1 + 1)) {
                                      a = (a + 1);
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      {
        const float x_123 = gl_FragCoord.x;
        if ((x_123 < -1.0f)) {
        } else {
          break;
        }
      }
    }
    {
      const float x_126 = gl_FragCoord.y;
      if ((x_126 < -1.0f)) {
      } else {
        break;
      }
    }
  }
  x_GLF_color = c;
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
