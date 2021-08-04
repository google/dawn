cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float4 func_() {
  const int x_48 = asint(x_6[0].x);
  if ((x_48 == 1)) {
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return float4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main_1() {
  int i = 0;
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  i = 0;
  while (true) {
    const int x_33 = i;
    const int x_35 = asint(x_6[0].x);
    if ((x_33 <= x_35)) {
    } else {
      break;
    }
    switch(i) {
      case 1: {
        const float4 x_43 = func_();
        x_GLF_color = x_43;
        /* fallthrough */
        {
          /* fallthrough */
        }
        {
          x_GLF_color.y = 0.0f;
        }
        break;
      }
      default: {
        /* fallthrough */
        {
          x_GLF_color.y = 0.0f;
        }
        break;
      }
      case 0: {
        x_GLF_color.y = 0.0f;
        break;
      }
    }
    {
      i = (i + 1);
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
