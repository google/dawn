cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[7];
};
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float sums[2] = (float[2])0;
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
  const int x_20 = asint(x_6[1].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_110 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
  sums[x_20] = x_110;
  const int x_22 = asint(x_6[2].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_114 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  sums[x_22] = x_114;
  const int x_23 = asint(x_6[1].x);
  a = x_23;
  while (true) {
    const int x_24 = a;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_25 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if ((x_24 < x_25)) {
    } else {
      break;
    }
    const int x_26 = asint(x_6[5].x);
    b = x_26;
    while (true) {
      const int x_27 = b;
      const int x_28 = asint(x_6[3].x);
      if ((x_27 < x_28)) {
      } else {
        break;
      }
      const int x_29 = asint(x_6[6].x);
      c = x_29;
      while (true) {
        const int x_30 = c;
        const int x_31 = asint(x_6[4].x);
        if ((x_30 <= x_31)) {
        } else {
          break;
        }
        const int x_32 = asint(x_6[1].x);
        d = x_32;
        while (true) {
          const int x_33 = d;
          const int x_34 = asint(x_6[6].x);
          if ((x_33 < x_34)) {
          } else {
            break;
          }
          const uint scalar_offset_3 = ((16u * uint(0))) / 4;
          const int x_35 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
          e = x_35;
          while (true) {
            const int x_36 = e;
            const int x_37 = asint(x_6[4].x);
            if ((x_36 <= x_37)) {
            } else {
              break;
            }
            const int x_38 = asint(x_6[1].x);
            f = x_38;
            while (true) {
              const int x_39 = f;
              const uint scalar_offset_4 = ((16u * uint(0))) / 4;
              const int x_40 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
              if ((x_39 < x_40)) {
              } else {
                break;
              }
              const int x_41 = asint(x_6[1].x);
              g = x_41;
              while (true) {
                const int x_42 = g;
                const int x_43 = asint(x_6[6].x);
                if ((x_42 < x_43)) {
                } else {
                  break;
                }
                const int x_44 = asint(x_6[1].x);
                h = x_44;
                while (true) {
                  const int x_45 = h;
                  const uint scalar_offset_5 = ((16u * uint(0))) / 4;
                  const int x_46 = asint(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
                  if ((x_45 < x_46)) {
                  } else {
                    break;
                  }
                  const int x_47 = asint(x_6[1].x);
                  i = x_47;
                  while (true) {
                    const int x_48 = i;
                    const int x_49 = asint(x_6[4].x);
                    if ((x_48 < x_49)) {
                    } else {
                      break;
                    }
                    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
                    const int x_50 = asint(x_6[scalar_offset_6 / 4][scalar_offset_6 % 4]);
                    j = x_50;
                    while (true) {
                      const int x_51 = j;
                      const int x_52 = asint(x_6[1].x);
                      if ((x_51 > x_52)) {
                      } else {
                        break;
                      }
                      const int x_53 = a;
                      const float x_197 = asfloat(x_8[2].x);
                      const float x_199 = sums[x_53];
                      sums[x_53] = (x_199 + x_197);
                      {
                        j = (j - 1);
                      }
                    }
                    {
                      i = (i + 1);
                    }
                  }
                  {
                    h = (h + 1);
                  }
                }
                {
                  g = (g + 1);
                }
              }
              {
                f = (f + 1);
              }
            }
            {
              e = (e + 1);
            }
          }
          {
            d = (d + 1);
          }
        }
        {
          c = (c + 1);
        }
      }
      {
        b = (b + 1);
      }
    }
    {
      a = (a + 1);
    }
  }
  const int x_74 = asint(x_6[1].x);
  const float x_204 = sums[x_74];
  const float x_206 = asfloat(x_8[1].x);
  const bool x_207 = (x_204 == x_206);
  x_216_phi = x_207;
  if (x_207) {
    const int x_75 = asint(x_6[2].x);
    const float x_212 = sums[x_75];
    const float x_214 = asfloat(x_8[1].x);
    x_215 = (x_212 == x_214);
    x_216_phi = x_215;
  }
  if (x_216_phi) {
    const int x_76 = asint(x_6[2].x);
    const int x_77 = asint(x_6[1].x);
    const int x_78 = asint(x_6[1].x);
    const int x_79 = asint(x_6[2].x);
    x_GLF_color = float4(float(x_76), float(x_77), float(x_78), float(x_79));
  } else {
    const int x_80 = asint(x_6[1].x);
    const float x_230 = float(x_80);
    x_GLF_color = float4(x_230, x_230, x_230, x_230);
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
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
