cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float a = 0.0f;
  const float x_30 = asfloat(x_6[1].x);
  a = x_30;
  const float x_32 = asfloat(x_6[3].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_34 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  if ((x_32 > x_34)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_39 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    a = (a + x_39);
    const float x_42 = a;
    x_GLF_color = float4(x_42, x_42, x_42, x_42);
    const float x_45 = asfloat(x_6[3].x);
    const float x_47 = asfloat(x_6[1].x);
    if ((x_45 > x_47)) {
      const float x_52 = x_GLF_color.x;
      a = (a + x_52);
      const float x_56 = asfloat(x_6[2].x);
      x_GLF_color = float4(x_56, x_56, x_56, x_56);
    }
  }
  x_GLF_color = float4(a, 0.0f, 0.0f, 1.0f);
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
