cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float arr[3] = (float[3])0;
  int a = 0;
  bool x_69 = false;
  bool x_79 = false;
  bool x_70_phi = false;
  bool x_80_phi = false;
  const float x_34 = asfloat(x_6[1].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_36 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const float x_38 = asfloat(x_6[2].x);
  const float tint_symbol_3[3] = {x_34, x_36, x_38};
  arr = tint_symbol_3;
  a = 0;
  while (true) {
    const int x_44 = a;
    const int x_46 = asint(x_9[1].x);
    if ((x_44 <= x_46)) {
    } else {
      break;
    }
    const int x_49 = a;
    a = (x_49 + 1);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_52 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    arr[x_49] = x_52;
  }
  const int x_55 = asint(x_9[1].x);
  const float x_57 = arr[x_55];
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_59 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const bool x_60 = (x_57 == x_59);
  x_70_phi = x_60;
  if (x_60) {
    const int x_64 = asint(x_9[2].x);
    const float x_66 = arr[x_64];
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const float x_68 = asfloat(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_69 = (x_66 == x_68);
    x_70_phi = x_69;
  }
  const bool x_70 = x_70_phi;
  x_80_phi = x_70;
  if (x_70) {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_74 = asint(x_9[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const float x_76 = arr[x_74];
    const float x_78 = asfloat(x_6[2].x);
    x_79 = (x_76 == x_78);
    x_80_phi = x_79;
  }
  if (x_80_phi) {
    const int x_85 = asint(x_9[1].x);
    const float x_87 = arr[x_85];
    const float x_89 = asfloat(x_6[1].x);
    const float x_91 = asfloat(x_6[1].x);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const float x_93 = asfloat(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    x_GLF_color = float4(x_87, x_89, x_91, x_93);
  } else {
    const float x_96 = asfloat(x_6[1].x);
    x_GLF_color = float4(x_96, x_96, x_96, x_96);
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
