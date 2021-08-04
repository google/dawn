cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_8 : register(b2, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

void main_1() {
  float f = 0.0f;
  const float x_37 = asfloat(x_6[1].x);
  const float x_39 = asfloat(x_8[0].x);
  const float x_42 = asfloat(x_6[2].x);
  const float x_44 = asfloat(x_8[0].x);
  const float x_49 = asfloat(x_8[0].y);
  f = (((x_37 * x_39) + (x_42 * round(x_44))) + x_49);
  const float x_51 = f;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_53 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  if ((x_51 == x_53)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_59 = asint(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_62 = asint(x_10[1].x);
    const int x_65 = asint(x_10[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_68 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_GLF_color = float4(float(x_59), float(x_62), float(x_65), float(x_68));
  } else {
    const int x_72 = asint(x_10[1].x);
    const float x_73 = float(x_72);
    x_GLF_color = float4(x_73, x_73, x_73, x_73);
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
