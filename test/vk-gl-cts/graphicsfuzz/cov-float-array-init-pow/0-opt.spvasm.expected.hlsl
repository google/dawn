cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[2];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  float arr[10] = (float[10])0;
  f = 2.0f;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_37 = asfloat(x_7[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_39 = asfloat(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_41 = asfloat(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const float x_43 = asfloat(x_7[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const float x_45 = asfloat(x_7[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  const uint scalar_offset_5 = ((16u * uint(0))) / 4;
  const float x_47 = asfloat(x_7[scalar_offset_5 / 4][scalar_offset_5 % 4]);
  const uint scalar_offset_6 = ((16u * uint(0))) / 4;
  const float x_49 = asfloat(x_7[scalar_offset_6 / 4][scalar_offset_6 % 4]);
  const float x_50 = f;
  const float x_52 = asfloat(x_7[1].x);
  const uint scalar_offset_7 = ((16u * uint(0))) / 4;
  const float x_55 = asfloat(x_7[scalar_offset_7 / 4][scalar_offset_7 % 4]);
  const uint scalar_offset_8 = ((16u * uint(0))) / 4;
  const float x_57 = asfloat(x_7[scalar_offset_8 / 4][scalar_offset_8 % 4]);
  const float tint_symbol_3[10] = {x_37, x_39, x_41, x_43, x_45, x_47, x_49, pow(x_50, x_52), x_55, x_57};
  arr = tint_symbol_3;
  const uint scalar_offset_9 = ((16u * uint(0))) / 4;
  const int x_60 = asint(x_9[scalar_offset_9 / 4][scalar_offset_9 % 4]);
  const float x_62 = arr[x_60];
  const int x_65 = asint(x_9[3].x);
  if ((int(x_62) == x_65)) {
    const int x_71 = asint(x_9[1].x);
    const int x_74 = asint(x_9[2].x);
    const int x_77 = asint(x_9[2].x);
    const int x_80 = asint(x_9[1].x);
    x_GLF_color = float4(float(x_71), float(x_74), float(x_77), float(x_80));
  } else {
    const int x_84 = asint(x_9[2].x);
    const float x_85 = float(x_84);
    x_GLF_color = float4(x_85, x_85, x_85, x_85);
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
