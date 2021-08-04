static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};

void main_1() {
  bool GLF_live12c5 = false;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  while (true) {
    const float x_31 = asfloat(x_5[0].y);
    if ((x_31 < 0.0f)) {
      GLF_live12c5 = false;
      if (GLF_live12c5) {
        {
          if (false) {
          } else {
            break;
          }
        }
        continue;
      } else {
        {
          if (false) {
          } else {
            break;
          }
        }
        continue;
      }
      {
        if (false) {
        } else {
          break;
        }
      }
      continue;
    }
    break;
    {
      if (false) {
      } else {
        break;
      }
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
