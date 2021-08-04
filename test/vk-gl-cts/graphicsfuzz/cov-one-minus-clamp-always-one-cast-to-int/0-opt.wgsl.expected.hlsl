cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  int a = 0;
  f = 2.0f;
  a = int((1.0f - clamp(1.0f, 1.0f, f)));
  const int x_31 = a;
  const int x_33 = asint(x_7[1].x);
  if ((x_31 == x_33)) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_39 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
    const int x_42 = asint(x_7[1].x);
    const int x_45 = asint(x_7[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_48 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_GLF_color = float4(float(x_39), float(x_42), float(x_45), float(x_48));
  } else {
    const int x_52 = asint(x_7[1].x);
    const float x_53 = float(x_52);
    x_GLF_color = float4(x_53, x_53, x_53, x_53);
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
