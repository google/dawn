uint tint_pack2x16float(float2 param_0) {
  uint2 i = f32tof16(param_0);
  return i.x | (i.y << 16);
}

float4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}

cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[4];
};
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  uint a = 0u;
  float4 values = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 ref = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_85 = false;
  bool x_101 = false;
  bool x_117 = false;
  bool x_86_phi = false;
  bool x_102_phi = false;
  bool x_118_phi = false;
  a = tint_pack2x16float(float2(1.0f, 1.0f));
  values = tint_unpack4x8unorm(a);
  const float x_41 = asfloat(x_8[3].x);
  const float x_43 = asfloat(x_8[1].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_45 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
  const float x_48 = asfloat(x_8[3].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_50 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const float x_53 = asfloat(x_8[1].x);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_55 = asfloat(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  ref = float4(x_41, (x_43 / x_45), (x_48 / x_50), (x_53 / x_55));
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_59 = asint(x_10[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const float x_61 = values[x_59];
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const int x_63 = asint(x_10[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  const float x_65 = ref[x_63];
  const float x_69 = asfloat(x_8[2].x);
  const bool x_70 = (abs((x_61 - x_65)) < x_69);
  x_86_phi = x_70;
  if (x_70) {
    const int x_74 = asint(x_10[1].x);
    const float x_76 = values[x_74];
    const int x_78 = asint(x_10[1].x);
    const float x_80 = ref[x_78];
    const float x_84 = asfloat(x_8[2].x);
    x_85 = (abs((x_76 - x_80)) < x_84);
    x_86_phi = x_85;
  }
  const bool x_86 = x_86_phi;
  x_102_phi = x_86;
  if (x_86) {
    const int x_90 = asint(x_10[3].x);
    const float x_92 = values[x_90];
    const int x_94 = asint(x_10[3].x);
    const float x_96 = ref[x_94];
    const float x_100 = asfloat(x_8[2].x);
    x_101 = (abs((x_92 - x_96)) < x_100);
    x_102_phi = x_101;
  }
  const bool x_102 = x_102_phi;
  x_118_phi = x_102;
  if (x_102) {
    const int x_106 = asint(x_10[2].x);
    const float x_108 = values[x_106];
    const int x_110 = asint(x_10[2].x);
    const float x_112 = ref[x_110];
    const float x_116 = asfloat(x_8[2].x);
    x_117 = (abs((x_108 - x_112)) < x_116);
    x_118_phi = x_117;
  }
  if (x_118_phi) {
    const int x_123 = asint(x_10[1].x);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_126 = asint(x_10[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_129 = asint(x_10[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const int x_132 = asint(x_10[1].x);
    x_GLF_color = float4(float(x_123), float(x_126), float(x_129), float(x_132));
  } else {
    const uint scalar_offset_7 = ((16u * uint(0))) / 4;
    const int x_136 = asint(x_10[scalar_offset_7 / 4][scalar_offset_7 % 4]);
    const float x_137 = float(x_136);
    x_GLF_color = float4(x_137, x_137, x_137, x_137);
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
