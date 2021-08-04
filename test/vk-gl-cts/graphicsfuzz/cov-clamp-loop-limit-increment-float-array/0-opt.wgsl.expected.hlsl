cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float sums[3] = (float[3])0;
  int i = 0;
  float2x4 indexable = float2x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_40 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_42 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_44 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const float tint_symbol_3[3] = {x_40, x_42, x_44};
  sums = tint_symbol_3;
  i = 0;
  while (true) {
    const int x_50 = i;
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_52 = asint(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_54 = asint(x_9[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    if ((x_50 < clamp(x_52, x_54, 2))) {
    } else {
      break;
    }
    const int x_59 = asint(x_9[2].x);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const float x_61 = asfloat(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const int x_65 = i;
    const int x_67 = asint(x_9[1].x);
    indexable = float2x4(float4(x_61, 0.0f, 0.0f, 0.0f), float4(0.0f, x_61, 0.0f, 0.0f));
    const float x_69 = indexable[x_65][x_67];
    const float x_71 = sums[x_59];
    sums[x_59] = (x_71 + x_69);
    {
      i = (i + 1);
    }
  }
  const int x_77 = asint(x_9[2].x);
  const float x_79 = sums[x_77];
  const float x_81 = asfloat(x_6[1].x);
  if ((x_79 == x_81)) {
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_87 = asint(x_9[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const int x_90 = asint(x_9[1].x);
    const int x_93 = asint(x_9[1].x);
    const uint scalar_offset_7 = ((16u * uint(0))) / 4;
    const int x_96 = asint(x_9[scalar_offset_7 / 4][scalar_offset_7 % 4]);
    x_GLF_color = float4(float(x_87), float(x_90), float(x_93), float(x_96));
  } else {
    const int x_100 = asint(x_9[1].x);
    const float x_101 = float(x_100);
    x_GLF_color = float4(x_101, x_101, x_101, x_101);
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
