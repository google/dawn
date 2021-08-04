cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  a = -1;
  const int x_25 = asint(x_6[1].x);
  const int x_26 = a;
  const int x_29 = asint(x_6[1].x);
  if (((x_25 / x_26) < x_29)) {
    const int x_35 = asint(x_6[1].x);
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_38 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_41 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_44 = asint(x_6[1].x);
    x_GLF_color = float4(float(x_35), float(x_38), float(x_41), float(x_44));
  } else {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_48 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const float x_49 = float(x_48);
    x_GLF_color = float4(x_49, x_49, x_49, x_49);
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
