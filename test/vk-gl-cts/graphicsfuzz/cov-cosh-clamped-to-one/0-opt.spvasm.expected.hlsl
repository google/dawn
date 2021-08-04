cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[1];
};

void main_1() {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_33 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  v = clamp(cosh(float4(1.0f, 1.0f, 1.0f, 1.0f)), float4(x_33, x_33, x_33, x_33), float4(1.0f, 1.0f, 1.0f, 1.0f));
  const float x_38 = v.x;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_40 = asint(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_43 = asint(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const float x_46 = v.z;
  x_GLF_color = float4(x_38, float(x_40), float(x_43), x_46);
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
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
