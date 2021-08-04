cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  int GLF_dead5cols = 0;
  int GLF_dead5rows = 0;
  int GLF_dead5c = 0;
  int GLF_dead5r = 0;
  int msb10 = 0;
  float donor_replacementGLF_dead5sums[9] = (float[9])0;
  i = 0;
  while (true) {
    const int x_45 = i;
    const float x_47 = asfloat(x_6[0].x);
    if ((x_45 >= int(x_47))) {
      break;
    }
    const float x_53 = asfloat(x_6[0].y);
    if ((0.0f > x_53)) {
      GLF_dead5cols = 2;
      {
        for(; (GLF_dead5cols <= 4); GLF_dead5cols = (GLF_dead5cols + 1)) {
          GLF_dead5rows = 2;
          {
            for(; (GLF_dead5rows <= 4); GLF_dead5rows = (GLF_dead5rows + 1)) {
              GLF_dead5c = 0;
              {
                for(; (GLF_dead5c < GLF_dead5cols); GLF_dead5c = (GLF_dead5c + 1)) {
                  GLF_dead5r = 0;
                  {
                    for(; (GLF_dead5r < GLF_dead5rows); GLF_dead5r = (GLF_dead5r + 1)) {
                      switch(msb10) {
                        case 1:
                        case 8: {
                          const int x_96 = (((msb10 >= 0) & (msb10 < 9)) ? msb10 : 0);
                          const float x_98 = donor_replacementGLF_dead5sums[x_96];
                          donor_replacementGLF_dead5sums[x_96] = (x_98 + 1.0f);
                          break;
                        }
                        default: {
                          break;
                        }
                      }
                    }
                  }
                }
              }
              msb10 = (msb10 + 1);
            }
          }
        }
      }
    }
    i = (i + 1);
    {
      if ((i < 200)) {
      } else {
        break;
      }
    }
  }
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
