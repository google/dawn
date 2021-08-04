cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_25 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_29 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  a = ((1 >> asuint((x_25 << asuint(5)))) >> asuint(x_29));
  if ((a == 1)) {
    const int x_37 = asint(x_6[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_40 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_43 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_46 = asint(x_6[1].x);
    x_GLF_color = float4(float(x_37), float(x_40), float(x_43), float(x_46));
  } else {
    const float x_50 = float(a);
    x_GLF_color = float4(x_50, x_50, x_50, x_50);
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
