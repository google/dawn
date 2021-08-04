cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int x_28 = 0;
  int x_29 = 0;
  int x_28_phi = 0;
  int x_31_phi = 0;
  int x_42_phi = 0;
  const int x_24 = min(1, reversebits(1));
  const int x_26 = asint(x_5[3].x);
  x_28_phi = x_26;
  x_31_phi = 1;
  while (true) {
    int x_32 = 0;
    x_28 = x_28_phi;
    const int x_31 = x_31_phi;
    x_42_phi = x_28;
    if ((x_31 <= (x_24 - 1))) {
    } else {
      break;
    }
    x_29 = asint((x_28 + asint(x_31)));
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_38 = asint(x_5[scalar_offset / 4][scalar_offset % 4]);
    if ((x_38 == 1)) {
      x_42_phi = x_29;
      break;
    }
    {
      x_32 = (x_31 + 1);
      x_28_phi = x_29;
      x_31_phi = x_32;
    }
  }
  const int x_42 = x_42_phi;
  const int x_44 = asint(x_5[2].x);
  if ((x_42 == x_44)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_50 = asint(x_5[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const float x_51 = float(x_50);
    const int x_53 = asint(x_5[1].x);
    const float x_54 = float(x_53);
    x_GLF_color = float4(x_51, x_54, x_54, x_51);
  } else {
    const int x_57 = asint(x_5[1].x);
    const float x_58 = float(x_57);
    x_GLF_color = float4(x_58, x_58, x_58, x_58);
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
