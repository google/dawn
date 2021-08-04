uint tint_pack2x16unorm(float2 param_0) {
  uint2 i = uint2(round(clamp(param_0, 0.0, 1.0) * 65535.0));
  return (i.x | i.y << 16);
}

float4 tint_unpack4x8snorm(uint param_0) {
  int j = int(param_0);
  int4 i = int4(j << 24, j << 16, j << 8, j) >> 24;
  return clamp(float4(i) / 127.0, -1.0, 1.0);
}

cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[7];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  uint a = 0u;
  float4 v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float E = 0.0f;
  bool x_75 = false;
  bool x_92 = false;
  bool x_109 = false;
  bool x_76_phi = false;
  bool x_93_phi = false;
  bool x_110_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_41 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const float x_43 = asfloat(x_6[1].x);
  a = tint_pack2x16unorm(float2(x_41, x_43));
  v1 = tint_unpack4x8snorm(a);
  E = 0.01f;
  const int x_49 = asint(x_10[2].x);
  const float x_51 = v1[x_49];
  const float x_53 = asfloat(x_6[2].x);
  const float x_55 = asfloat(x_6[3].x);
  const bool x_60 = (abs((x_51 - (x_53 / x_55))) < E);
  x_76_phi = x_60;
  if (x_60) {
    const int x_64 = asint(x_10[1].x);
    const float x_66 = v1[x_64];
    const float x_68 = asfloat(x_6[4].x);
    const float x_70 = asfloat(x_6[3].x);
    x_75 = (abs((x_66 - (x_68 / x_70))) < E);
    x_76_phi = x_75;
  }
  const bool x_76 = x_76_phi;
  x_93_phi = x_76;
  if (x_76) {
    const int x_80 = asint(x_10[3].x);
    const float x_82 = v1[x_80];
    const float x_84 = asfloat(x_6[5].x);
    const float x_87 = asfloat(x_6[3].x);
    x_92 = (abs((x_82 - (-(x_84) / x_87))) < E);
    x_93_phi = x_92;
  }
  const bool x_93 = x_93_phi;
  x_110_phi = x_93;
  if (x_93) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_97 = asint(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const float x_99 = v1[x_97];
    const float x_101 = asfloat(x_6[6].x);
    const float x_104 = asfloat(x_6[3].x);
    x_109 = (abs((x_99 - (-(x_101) / x_104))) < E);
    x_110_phi = x_109;
  }
  if (x_110_phi) {
    const int x_115 = asint(x_10[1].x);
    const int x_118 = asint(x_10[2].x);
    const int x_121 = asint(x_10[2].x);
    const int x_124 = asint(x_10[1].x);
    x_GLF_color = float4(float(x_115), float(x_118), float(x_121), float(x_124));
  } else {
    const float x_128 = asfloat(x_6[5].x);
    x_GLF_color = float4(x_128, x_128, x_128, x_128);
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
