cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int x_23 = 0;
  int x_27 = 0;
  int x_37 = 0;
  int x_23_phi = 0;
  int x_45_phi = 0;
  x_23_phi = 0;
  while (true) {
    int x_24 = 0;
    x_23 = x_23_phi;
    x_27 = asint(x_5[1].x);
    if ((x_23 < (100 - asint(x_27)))) {
    } else {
      break;
    }
    {
      x_24 = asint((x_23 + asint(1)));
      x_23_phi = x_24;
    }
  }
  int x_37_phi = 0;
  int x_40_phi = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_32 = asint(x_5[scalar_offset / 4][scalar_offset % 4]);
  x_45_phi = 1;
  if ((x_32 == 0)) {
    x_37_phi = 1;
    x_40_phi = x_23;
    while (true) {
      int x_41 = 0;
      int x_38 = 0;
      x_37 = x_37_phi;
      const int x_40 = x_40_phi;
      if ((x_40 < 100)) {
      } else {
        break;
      }
      {
        x_41 = (x_40 + 1);
        x_38 = asint((x_37 * asint((1 - asint(x_37)))));
        x_37_phi = x_38;
        x_40_phi = x_41;
      }
    }
    x_45_phi = x_37;
  }
  if ((x_45_phi == x_32)) {
    const float x_50 = float(x_27);
    const float x_51 = float(x_32);
    x_GLF_color = float4(x_50, x_51, x_51, x_50);
  } else {
    const float x_53 = float(x_32);
    x_GLF_color = float4(x_53, x_53, x_53, x_53);
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
