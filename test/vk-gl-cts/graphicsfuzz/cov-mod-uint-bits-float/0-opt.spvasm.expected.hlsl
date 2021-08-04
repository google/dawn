static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};

void main_1() {
  float a = 0.0f;
  a = (asfloat(1u) - (1.0f * floor((asfloat(1u) / 1.0f))));
  const float x_29 = asfloat(x_6[1].x);
  x_GLF_color = float4(x_29, x_29, x_29, x_29);
  const float x_31 = a;
  const float x_33 = asfloat(x_6[2].x);
  if ((x_31 < x_33)) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const float x_38 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
    const float x_40 = asfloat(x_6[1].x);
    const float x_42 = asfloat(x_6[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_44 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_GLF_color = float4(x_38, x_40, x_42, x_44);
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
