cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_24 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_26 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  a = max(x_24, max(x_26, 1));
  const int x_29 = a;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_31 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  if ((x_29 == x_31)) {
    const int x_36 = a;
    const int x_39 = asint(x_6[1].x);
    const int x_42 = asint(x_6[1].x);
    x_GLF_color = float4(float(x_36), float(x_39), float(x_42), float(a));
  } else {
    const float x_48 = float(a);
    x_GLF_color = float4(x_48, x_48, x_48, x_48);
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
