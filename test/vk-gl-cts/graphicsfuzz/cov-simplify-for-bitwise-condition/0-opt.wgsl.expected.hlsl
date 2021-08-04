cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int i = 0;
  const int x_25 = asint(x_6[1].x);
  a = x_25;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_27 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  i = -(x_27);
  while (true) {
    const int x_33 = i;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_35 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_38 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if (((x_33 | x_35) < x_38)) {
    } else {
      break;
    }
    const int x_41 = i;
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_43 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    a = (x_41 * x_43);
    {
      i = (i + 1);
    }
  }
  const int x_47 = a;
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const int x_49 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  if ((x_47 == -(x_49))) {
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_56 = asint(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const int x_59 = asint(x_6[1].x);
    const int x_62 = asint(x_6[1].x);
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_65 = asint(x_6[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    x_GLF_color = float4(float(x_56), float(x_59), float(x_62), float(x_65));
  } else {
    const float x_69 = float(a);
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
