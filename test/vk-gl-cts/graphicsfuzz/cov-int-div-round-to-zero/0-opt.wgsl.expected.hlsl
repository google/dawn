cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  const int x_28 = asint(x_6[1].x);
  a = x_28;
  const int x_29 = a;
  const int x_31 = asint(x_6[2].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_37 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  if (((int2(x_29, x_29) / int2(x_31, 63677)).y == x_37)) {
    const int x_43 = asint(x_6[2].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_46 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_49 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_52 = asint(x_6[2].x);
    x_GLF_color = float4(float(x_43), float(x_46), float(x_49), float(x_52));
  } else {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_56 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_57 = float(x_56);
    x_GLF_color = float4(x_57, x_57, x_57, x_57);
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
