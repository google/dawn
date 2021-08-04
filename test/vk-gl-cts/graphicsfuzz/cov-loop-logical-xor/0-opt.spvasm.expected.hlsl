cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_26 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  a = x_26;
  const int x_28 = asint(x_6[1].x);
  const float x_29 = float(x_28);
  x_GLF_color = float4(x_29, x_29, x_29, x_29);
  while (true) {
    const int x_36 = asint(x_6[2].x);
    if (((x_36 == a) != true)) {
    } else {
      break;
    }
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_42 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_45 = asint(x_6[1].x);
    const int x_48 = asint(x_6[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_51 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_GLF_color = float4(float(x_42), float(x_45), float(x_48), float(x_51));
    break;
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
