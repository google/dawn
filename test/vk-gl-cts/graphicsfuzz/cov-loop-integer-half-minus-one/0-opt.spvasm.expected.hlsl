cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  const int x_25 = asint(x_6[2].x);
  a = x_25;
  while (true) {
    if ((a >= 0)) {
    } else {
      break;
    }
    a = ((a / 2) - 1);
  }
  const int x_36 = a;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_38 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  if ((x_36 == -(x_38))) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_45 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_48 = asint(x_6[1].x);
    const int x_51 = asint(x_6[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_54 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_GLF_color = float4(float(x_45), float(x_48), float(x_51), float(x_54));
  } else {
    const int x_58 = asint(x_6[1].x);
    const float x_59 = float(x_58);
    x_GLF_color = float4(x_59, x_59, x_59, x_59);
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
