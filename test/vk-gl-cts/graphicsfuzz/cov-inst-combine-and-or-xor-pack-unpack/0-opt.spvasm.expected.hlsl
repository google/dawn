uint tint_pack4x8unorm(float4 param_0) {
  uint4 i = uint4(round(clamp(param_0, 0.0, 1.0) * 255.0));
  return (i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

float4 tint_unpack4x8snorm(uint param_0) {
  int j = int(param_0);
  int4 i = int4(j << 24, j << 16, j << 8, j) >> 24;
  return clamp(float4(i) / 127.0, -1.0, 1.0);
}

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  uint a = 0u;
  float4 v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 ref = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_85 = false;
  bool x_97 = false;
  bool x_109 = false;
  bool x_86_phi = false;
  bool x_98_phi = false;
  bool x_110_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_36 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  a = tint_pack4x8unorm(float4(x_36, x_36, x_36, x_36));
  v1 = tint_unpack4x8snorm(a);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_42 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const float x_45 = asfloat(x_6[1].x);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_48 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const float x_51 = asfloat(x_6[1].x);
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const float x_54 = asfloat(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const float x_57 = asfloat(x_6[1].x);
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const float x_60 = asfloat(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  const float x_63 = asfloat(x_6[1].x);
  ref = float4((-(x_42) / x_45), (-(x_48) / x_51), (-(x_54) / x_57), (-(x_60) / x_63));
  const int x_67 = asint(x_10[1].x);
  const float x_69 = v1[x_67];
  const uint scalar_offset_5 = ((16u * uint(0))) / 4;
  const int x_71 = asint(x_10[scalar_offset_5 / 4][scalar_offset_5 % 4]);
  const float x_73 = ref[x_71];
  const bool x_74 = (x_69 == x_73);
  x_86_phi = x_74;
  if (x_74) {
    const int x_78 = asint(x_10[3].x);
    const float x_80 = v1[x_78];
    const int x_82 = asint(x_10[2].x);
    const float x_84 = ref[x_82];
    x_85 = (x_80 == x_84);
    x_86_phi = x_85;
  }
  const bool x_86 = x_86_phi;
  x_98_phi = x_86;
  if (x_86) {
    const int x_90 = asint(x_10[2].x);
    const float x_92 = v1[x_90];
    const int x_94 = asint(x_10[3].x);
    const float x_96 = ref[x_94];
    x_97 = (x_92 == x_96);
    x_98_phi = x_97;
  }
  const bool x_98 = x_98_phi;
  x_110_phi = x_98;
  if (x_98) {
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_102 = asint(x_10[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const float x_104 = v1[x_102];
    const int x_106 = asint(x_10[1].x);
    const float x_108 = ref[x_106];
    x_109 = (x_104 == x_108);
    x_110_phi = x_109;
  }
  if (x_110_phi) {
    const int x_115 = asint(x_10[3].x);
    const int x_118 = asint(x_10[1].x);
    const int x_121 = asint(x_10[1].x);
    const int x_124 = asint(x_10[3].x);
    x_GLF_color = float4(float(x_115), float(x_118), float(x_121), float(x_124));
  } else {
    const int x_128 = asint(x_10[1].x);
    const float x_130 = v1[x_128];
    x_GLF_color = float4(x_130, x_130, x_130, x_130);
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
