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
      [loop] while (true) {
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
C:\src\tint\test\Shader@0x0000013B8CBD6FE0(26,13-21): error X3708: continue cannot be used in a switch

