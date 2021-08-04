cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int i = 0;
  const int x_26 = asint(x_6[1].x);
  a = x_26;
  const int x_28 = asint(x_6[1].x);
  i = x_28;
  while (true) {
    const int x_33 = i;
    const int x_35 = asint(x_6[2].x);
    if ((x_33 < x_35)) {
    } else {
      break;
    }
    if ((~(i) != 0)) {
      a = (a + 1);
    }
    {
      i = (i + 1);
    }
  }
  const int x_47 = a;
  const int x_49 = asint(x_6[2].x);
  if ((x_47 == x_49)) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_55 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    const int x_58 = asint(x_6[1].x);
    const int x_61 = asint(x_6[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_64 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_GLF_color = float4(float(x_55), float(x_58), float(x_61), float(x_64));
  } else {
    const int x_68 = asint(x_6[1].x);
    const float x_69 = float(x_68);
    x_GLF_color = float4(x_69, x_69, x_69, x_69);
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
