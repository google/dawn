cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int sum = 0;
  int i = 0;
  a = 65536;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_29 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
  sum = x_29;
  const int x_31 = asint(x_7[1].x);
  if ((1 == x_31)) {
    a = (a - 1);
  }
  i = 0;
  while (true) {
    if ((i < a)) {
    } else {
      break;
    }
    sum = (sum + i);
    {
      const int x_49 = asint(x_7[2].x);
      i = (i + x_49);
    }
  }
  const int x_52 = sum;
  const int x_54 = asint(x_7[3].x);
  if ((x_52 == x_54)) {
    const int x_60 = asint(x_7[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_63 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_66 = asint(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_69 = asint(x_7[1].x);
    x_GLF_color = float4(float(x_60), float(x_63), float(x_66), float(x_69));
  } else {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_73 = asint(x_7[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_74 = float(x_73);
    x_GLF_color = float4(x_74, x_74, x_74, x_74);
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
