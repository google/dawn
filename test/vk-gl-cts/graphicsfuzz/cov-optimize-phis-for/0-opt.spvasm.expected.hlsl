cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[2];
};
cbuffer cbuffer_x_11 : register(b1, space0) {
  uint4 x_11[3];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

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
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_104 = asfloat(x_7[scalar_offset / 4][scalar_offset % 4]);
  a = x_104;
  const float x_106 = asfloat(x_7[1].x);
  b = x_106;
  const int x_24 = asint(x_11[1].x);
  i = x_24;
  while (true) {
    const int x_25 = i;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_26 = asint(x_11[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    if ((x_25 < x_26)) {
    } else {
      break;
    }
    const int x_27 = asint(x_11[1].x);
    i_1 = x_27;
    while (true) {
      const int x_28 = i_1;
      const uint scalar_offset_2 = ((16u * uint(0))) / 4;
      const int x_29 = asint(x_11[scalar_offset_2 / 4][scalar_offset_2 % 4]);
      if ((x_28 < x_29)) {
      } else {
        break;
      }
      const int x_30 = asint(x_11[1].x);
      i_2 = x_30;
      while (true) {
        const int x_31 = i_2;
        const uint scalar_offset_3 = ((16u * uint(0))) / 4;
        const int x_32 = asint(x_11[scalar_offset_3 / 4][scalar_offset_3 % 4]);
        if ((x_31 < x_32)) {
        } else {
          break;
        }
        const int x_33 = asint(x_11[2].x);
        i_3 = x_33;
        while (true) {
          const int x_34 = i_3;
          const uint scalar_offset_4 = ((16u * uint(0))) / 4;
          const int x_35 = asint(x_11[scalar_offset_4 / 4][scalar_offset_4 % 4]);
          if ((x_34 < x_35)) {
          } else {
            break;
          }
          const int x_36 = asint(x_11[2].x);
          i_4 = x_36;
          while (true) {
            const int x_37 = i_4;
            const uint scalar_offset_5 = ((16u * uint(0))) / 4;
            const int x_38 = asint(x_11[scalar_offset_5 / 4][scalar_offset_5 % 4]);
            if ((x_37 < x_38)) {
            } else {
              break;
            }
            const int x_39 = asint(x_11[1].x);
            i_5 = x_39;
            while (true) {
              const int x_40 = i_5;
              const uint scalar_offset_6 = ((16u * uint(0))) / 4;
              const int x_41 = asint(x_11[scalar_offset_6 / 4][scalar_offset_6 % 4]);
              if ((x_40 < x_41)) {
              } else {
                break;
              }
              const int x_42 = asint(x_11[1].x);
              i_6 = x_42;
              while (true) {
                const int x_43 = i_6;
                const uint scalar_offset_7 = ((16u * uint(0))) / 4;
                const int x_44 = asint(x_11[scalar_offset_7 / 4][scalar_offset_7 % 4]);
                if ((x_43 < x_44)) {
                } else {
                  break;
                }
                const int x_45 = asint(x_11[1].x);
                i_7 = x_45;
                while (true) {
                  const int x_46 = i_7;
                  const uint scalar_offset_8 = ((16u * uint(0))) / 4;
                  const int x_47 = asint(x_11[scalar_offset_8 / 4][scalar_offset_8 % 4]);
                  if ((x_46 < x_47)) {
                  } else {
                    break;
                  }
                  const int x_48 = asint(x_11[1].x);
                  i_8 = x_48;
                  while (true) {
                    const int x_49 = i_8;
                    const uint scalar_offset_9 = ((16u * uint(0))) / 4;
                    const int x_50 = asint(x_11[scalar_offset_9 / 4][scalar_offset_9 % 4]);
                    if ((x_49 < x_50)) {
                    } else {
                      break;
                    }
                    const int x_51 = asint(x_11[1].x);
                    i_9 = x_51;
                    while (true) {
                      const int x_52 = i_9;
                      const uint scalar_offset_10 = ((16u * uint(0))) / 4;
                      const int x_53 = asint(x_11[scalar_offset_10 / 4][scalar_offset_10 % 4]);
                      if ((x_52 < x_53)) {
                      } else {
                        break;
                      }
                      const int x_54 = asint(x_11[1].x);
                      i_10 = x_54;
                      while (true) {
                        const int x_55 = i_10;
                        const uint scalar_offset_11 = ((16u * uint(0))) / 4;
                        const int x_56 = asint(x_11[scalar_offset_11 / 4][scalar_offset_11 % 4]);
                        if ((x_55 < x_56)) {
                        } else {
                          break;
                        }
                        const float x_196 = asfloat(x_7[1].x);
                        a = x_196;
                        const float x_198 = gl_FragCoord.y;
                        const float x_200 = asfloat(x_7[1].x);
                        if ((x_198 > x_200)) {
                          break;
                        }
                        {
                          i_10 = (i_10 + 1);
                        }
                      }
                      {
                        i_9 = (i_9 + 1);
                      }
                    }
                    {
                      i_8 = (i_8 + 1);
                    }
                  }
                  {
                    i_7 = (i_7 + 1);
                  }
                }
                {
                  i_6 = (i_6 + 1);
                }
              }
              {
                i_5 = (i_5 + 1);
              }
            }
            {
              i_4 = (i_4 + 1);
            }
          }
          {
            i_3 = (i_3 + 1);
          }
        }
        {
          i_2 = (i_2 + 1);
        }
      }
      {
        i_1 = (i_1 + 1);
      }
    }
    b = (b + 1.0f);
    {
      i = (i + 1);
    }
  }
  x_GLF_color = float4(b, a, a, b);
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
  const main_out tint_symbol_5 = {x_GLF_color};
  return tint_symbol_5;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
