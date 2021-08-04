cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int i = 0;
  const int x_27 = asint(x_6[1].x);
  a = x_27;
  const int x_29 = asint(x_6[3].x);
  i = -(x_29);
  while (true) {
    const int x_36 = (i + 1);
    i = x_36;
    const int x_39 = asint(x_6[2].x);
    if ((reversebits(x_36) <= x_39)) {
    } else {
      break;
    }
    a = (a + 1);
  }
  const int x_44 = a;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_46 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  if ((x_44 == x_46)) {
    const int x_52 = asint(x_6[2].x);
    const int x_55 = asint(x_6[1].x);
    const int x_58 = asint(x_6[1].x);
    const int x_61 = asint(x_6[2].x);
    x_GLF_color = float4(float(x_52), float(x_55), float(x_58), float(x_61));
  } else {
    const int x_65 = asint(x_6[1].x);
    const float x_66 = float(x_65);
    x_GLF_color = float4(x_66, x_66, x_66, x_66);
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
