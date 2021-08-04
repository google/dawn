static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[2];
};

void main_1() {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_25 = asint(x_5[scalar_offset / 4][scalar_offset % 4]);
  const int x_28 = asint(x_5[1].x);
  const int x_31 = asint(x_5[1].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_34 = asint(x_5[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  x_GLF_color = float4(float(x_25), float(x_28), float(x_31), float(x_34));
  v = x_GLF_color;
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x_GLF_color = v;
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
