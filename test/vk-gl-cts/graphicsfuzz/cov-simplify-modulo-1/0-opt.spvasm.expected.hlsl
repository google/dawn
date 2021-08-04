cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[1];
};

void main_1() {
  float a = 0.0f;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_30 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  a = (x_30 - (1.0f * floor((x_30 / 1.0f))));
  const float x_32 = a;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_34 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_32 == x_34)) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_40 = asint(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const float x_42 = a;
    const float x_43 = a;
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_45 = asint(x_8[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_40), x_42, x_43, float(x_45));
  } else {
    const float x_48 = a;
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
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
