cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  int value = 0;
  float y = 0.0f;
  int x_31_phi = 0;
  i = 0;
  x_31_phi = 0;
  while (true) {
    const int x_31 = x_31_phi;
    const float x_37 = asfloat(x_6[0].x);
    if ((x_31 < (2 + int(x_37)))) {
    } else {
      break;
    }
    float x_55_phi = 0.0f;
    float x_46_phi = 0.0f;
    value = x_31;
    y = 0.5f;
    x_55_phi = 0.5f;
    x_46_phi = 0.5f;
    switch(x_31) {
      case 0: {
        const float x_54 = (0.5f + 0.5f);
        y = x_54;
        x_55_phi = x_54;
        /* fallthrough */
        {
          const float x_47 = clamp(1.0f, 0.5f, x_55_phi);
          y = x_47;
          x_46_phi = x_47;
          /* fallthrough */
        }
        {
          /* fallthrough */
        }
        {
          if ((x_46_phi == 1.0f)) {
            x_GLF_color = float4(float((x_31 + 1)), 0.0f, 0.0f, 1.0f);
            return;
          }
        }
        break;
      }
      case 1: {
        const float x_47 = clamp(1.0f, 0.5f, x_55_phi);
        y = x_47;
        x_46_phi = x_47;
        /* fallthrough */
        {
          /* fallthrough */
        }
        {
          if ((x_46_phi == 1.0f)) {
            x_GLF_color = float4(float((x_31 + 1)), 0.0f, 0.0f, 1.0f);
            return;
          }
        }
        break;
      }
      default: {
        /* fallthrough */
        {
          if ((x_46_phi == 1.0f)) {
            x_GLF_color = float4(float((x_31 + 1)), 0.0f, 0.0f, 1.0f);
            return;
          }
        }
        break;
      }
      case 2: {
        if ((x_46_phi == 1.0f)) {
          x_GLF_color = float4(float((x_31 + 1)), 0.0f, 0.0f, 1.0f);
          return;
        }
        break;
      }
    }
    {
      const int x_32 = (x_31 + 1);
      i = x_32;
      x_31_phi = x_32;
    }
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
