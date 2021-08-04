cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int i = 0;
  int a = 0;
  float4 indexable[2] = (float4[2])0;
  float4 indexable_1[2] = (float4[2])0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_45 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  v1 = float4(x_45, x_45, x_45, x_45);
  const int x_48 = asint(x_9[1].x);
  i = x_48;
  while (true) {
    const int x_53 = i;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_55 = asint(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    if ((x_53 < x_55)) {
    } else {
      break;
    }
    const int x_58 = i;
    const int x_60 = asint(x_9[1].x);
    const int x_62 = asint(x_9[2].x);
    const float4 tint_symbol_3[2] = {float4(1.0f, 1.0f, 1.0f, 1.0f), float4(0.0f, 0.0f, 0.0f, 0.0f)};
    indexable = tint_symbol_3;
    const float x_65 = indexable[clamp(x_58, x_60, x_62)].x;
    a = int(x_65);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_68 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const float x_70 = asfloat(x_6[1].x);
    const float x_72 = asfloat(x_6[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const float x_74 = asfloat(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_77 = asfloat(x_6[1].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const float x_79 = asfloat(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const float x_81 = asfloat(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const float x_83 = asfloat(x_6[1].x);
    const int x_86 = a;
    const float4 tint_symbol_4[2] = {float4(x_68, x_70, x_72, x_74), float4(x_77, x_79, x_81, x_83)};
    indexable_1 = tint_symbol_4;
    const float4 x_88 = indexable_1[x_86];
    v1 = x_88;
    {
      i = (i + 1);
    }
  }
  x_GLF_color = v1;
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
  const main_out tint_symbol_5 = {x_GLF_color};
  return tint_symbol_5;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
