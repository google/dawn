cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float A1[3] = (float[3])0;
  int a = 0;
  float b = 0.0f;
  bool c = false;
  bool x_36 = false;
  const float x_38 = asfloat(x_6[2].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_40 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const float x_42 = asfloat(x_6[1].x);
  const float tint_symbol_3[3] = {x_38, x_40, x_42};
  A1 = tint_symbol_3;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_45 = asint(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_47 = asint(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const int x_49 = asint(x_9[1].x);
  a = clamp(x_45, x_47, x_49);
  const int x_51 = a;
  const int x_53 = asint(x_9[1].x);
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_55 = asint(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const float x_58 = A1[clamp(x_51, x_53, x_55)];
  b = x_58;
  const float x_59 = b;
  const int x_61 = asint(x_9[1].x);
  const float x_63 = A1[x_61];
  if ((x_59 < x_63)) {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const float x_69 = asfloat(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const float x_71 = asfloat(x_6[2].x);
    x_36 = (x_69 > x_71);
  } else {
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const float x_74 = asfloat(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const int x_76 = asint(x_9[2].x);
    const float x_78 = A1[x_76];
    x_36 = (x_74 < x_78);
  }
  c = x_36;
  if (c) {
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_86 = asint(x_9[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const int x_89 = asint(x_9[1].x);
    const int x_92 = asint(x_9[1].x);
    const uint scalar_offset_7 = ((16u * uint(0))) / 4;
    const int x_95 = asint(x_9[scalar_offset_7 / 4][scalar_offset_7 % 4]);
    x_GLF_color = float4(float(x_86), float(x_89), float(x_92), float(x_95));
  } else {
    const uint scalar_offset_8 = ((16u * uint(0))) / 4;
    const float x_99 = asfloat(x_6[scalar_offset_8 / 4][scalar_offset_8 % 4]);
    x_GLF_color = float4(x_99, x_99, x_99, x_99);
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
