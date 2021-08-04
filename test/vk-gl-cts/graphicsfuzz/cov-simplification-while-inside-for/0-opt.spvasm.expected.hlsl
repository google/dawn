static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[1];
};

void main_1() {
  int i = 0;
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  i = 0;
  while (true) {
    const int x_38 = i;
    const int x_40 = asint(x_6[0].x);
    if ((x_38 < x_40)) {
    } else {
      break;
    }
    while (true) {
      const int x_48 = asint(x_6[0].x);
      if ((x_48 == 1)) {
        x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
      }
      {
        if (false) {
        } else {
          break;
        }
      }
    }
    {
      i = (i + 1);
    }
  }
  const int x_55 = asint(x_9[0].x);
  v.y = float(x_55);
  const float x_59 = v.y;
  x_GLF_color.y = x_59;
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
