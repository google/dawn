SKIP: FAILED

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  const float x_51 = asfloat(x_6[0].x);
  i = int(x_51);
  switch(i) {
    case 0: {
      while (true) {
        i = (i + 1);
        switch(i) {
          case 2: {
            i = (i + 5);
            break;
          }
          case 1: {
            {
              if ((i > 200)) {
              } else {
                break;
              }
            }
            continue;
            break;
          }
          default: {
            i = (i + 7);
            break;
          }
        }
        {
          if ((i > 200)) {
          } else {
            break;
          }
        }
      }
      if ((i > 100)) {
        i = (i - 2);
        break;
      }
      /* fallthrough */
      {
        i = (i - 3);
      }
      break;
    }
    default: {
      i = (i - 3);
      break;
    }
  }
  if ((i == -2)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_3 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_3;
}
O:\src\chrome\src\third_party\dawn\third_party\tint\test\Shader@0x0000028C1CDAABD0(26,13-21): error X3708: continue cannot be used in a switch

