cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

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
  while (true) {
    const int x_40 = i0;
    const int x_42 = asint(x_7[0].x);
    if ((x_40 < x_42)) {
    } else {
      break;
    }
    i1 = 0;
    while (true) {
      const int x_49 = i1;
      const int x_51 = asint(x_7[0].x);
      if ((x_49 < x_51)) {
      } else {
        break;
      }
      i2 = 0;
      while (true) {
        const int x_58 = i2;
        const int x_60 = asint(x_7[0].x);
        if ((x_58 < x_60)) {
        } else {
          break;
        }
        i3 = 0;
        while (true) {
          const int x_67 = i3;
          const int x_69 = asint(x_7[0].x);
          if ((x_67 < (x_69 + 2))) {
          } else {
            break;
          }
          i4 = 0;
          while (true) {
            const int x_77 = i4;
            const int x_79 = asint(x_7[0].x);
            if ((x_77 < x_79)) {
            } else {
              break;
            }
            i5 = 0;
            while (true) {
              const int x_86 = i5;
              const int x_88 = asint(x_7[0].x);
              if ((x_86 < x_88)) {
              } else {
                break;
              }
              while (true) {
                const int x_96 = asint(x_7[0].x);
                if ((x_96 > 0)) {
                } else {
                  break;
                }
                i6 = 0;
                while (true) {
                  const int x_103 = i6;
                  const int x_105 = asint(x_7[0].x);
                  if ((x_103 < x_105)) {
                  } else {
                    break;
                  }
                  i7 = 0;
                  while (true) {
                    const int x_112 = i7;
                    const int x_114 = asint(x_7[0].x);
                    if ((x_112 < x_114)) {
                    } else {
                      break;
                    }
                    i8_1 = 0;
                    while (true) {
                      const int x_121 = i8_1;
                      const int x_123 = asint(x_7[0].x);
                      if ((x_121 < x_123)) {
                      } else {
                        break;
                      }
                      i9 = 0;
                      while (true) {
                        const int x_130 = i9;
                        const int x_132 = asint(x_7[0].x);
                        if ((x_130 < x_132)) {
                        } else {
                          break;
                        }
                        a = (a + 1);
                        {
                          i9 = (i9 + 1);
                        }
                      }
                      {
                        i8_1 = (i8_1 + 1);
                      }
                    }
                    {
                      i7 = (i7 + 1);
                    }
                  }
                  {
                    i6 = (i6 + 1);
                  }
                }
                break;
              }
              {
                i5 = (i5 + 1);
              }
            }
            {
              i4 = (i4 + 1);
            }
          }
          {
            i3 = (i3 + 1);
          }
        }
        {
          i2 = (i2 + 1);
        }
      }
      {
        i1 = (i1 + 1);
      }
    }
    {
      i0 = (i0 + 1);
    }
  }
  if ((a == 3)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
