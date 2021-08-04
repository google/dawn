cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int b = 0;
  int c = 0;
  bool x_65 = false;
  bool x_66_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_29 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  a = x_29;
  const int x_31 = asint(x_6[1].x);
  b = x_31;
  const int x_33 = asint(x_6[2].x);
  c = x_33;
  while (true) {
    if ((a < b)) {
    } else {
      break;
    }
    a = (a + 1);
    const int x_44 = c;
    const int x_46 = asint(x_6[2].x);
    if ((x_44 == x_46)) {
      const int x_52 = asint(x_6[3].x);
      c = (c * x_52);
    } else {
      if (true) {
        continue;
      }
    }
  }
  const bool x_59 = (a == b);
  x_66_phi = x_59;
  if (x_59) {
    const int x_62 = c;
    const int x_64 = asint(x_6[3].x);
    x_65 = (x_62 == x_64);
    x_66_phi = x_65;
  }
  if (x_66_phi) {
    const int x_71 = asint(x_6[2].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_74 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_77 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_80 = asint(x_6[2].x);
    x_GLF_color = float4(float(x_71), float(x_74), float(x_77), float(x_80));
  } else {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_84 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_85 = float(x_84);
    x_GLF_color = float4(x_85, x_85, x_85, x_85);
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
