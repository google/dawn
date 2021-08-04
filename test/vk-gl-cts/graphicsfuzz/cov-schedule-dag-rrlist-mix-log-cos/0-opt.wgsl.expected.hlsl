cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[5];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  bool x_51 = false;
  bool x_52_phi = false;
  const float x_28 = asfloat(x_6[2].x);
  const float x_30 = asfloat(x_6[3].x);
  const float x_32 = asfloat(x_6[3].x);
  const float x_34 = asfloat(x_6[2].x);
  a = ((x_32 > x_34) ? x_30 : x_28);
  b = cos(log(a));
  const float x_40 = b;
  x_GLF_color = float4(x_40, x_40, x_40, x_40);
  const float x_42 = b;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_44 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const bool x_45 = (x_42 > x_44);
  x_52_phi = x_45;
  if (x_45) {
    const float x_48 = b;
    const float x_50 = asfloat(x_6[1].x);
    x_51 = (x_48 < x_50);
    x_52_phi = x_51;
  }
  if (x_52_phi) {
    const float x_56 = asfloat(x_6[3].x);
    const float x_58 = asfloat(x_6[4].x);
    const float x_60 = asfloat(x_6[4].x);
    const float x_62 = asfloat(x_6[3].x);
    x_GLF_color = float4(x_56, x_58, x_60, x_62);
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
