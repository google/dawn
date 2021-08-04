uint tint_pack4x8snorm(float4 param_0) {
  int4 i = int4(round(clamp(param_0, -1.0, 1.0) * 127.0)) & 0xff;
  return asuint(i.x | i.y << 8 | i.z << 16 | i.w << 24);
}

float4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}

cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[4];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  uint a = 0u;
  float4 v1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float E = 0.0f;
  bool x_69 = false;
  bool x_85 = false;
  bool x_101 = false;
  bool x_70_phi = false;
  bool x_86_phi = false;
  bool x_102_phi = false;
  const float x_35 = asfloat(x_6[1].x);
  a = tint_pack4x8snorm(float4(x_35, x_35, x_35, x_35));
  v1 = tint_unpack4x8unorm(a);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_41 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  E = x_41;
  const int x_43 = asint(x_10[1].x);
  const float x_45 = v1[x_43];
  const float x_47 = asfloat(x_6[2].x);
  const float x_49 = asfloat(x_6[3].x);
  const bool x_54 = (abs((x_45 - (x_47 / x_49))) < E);
  x_70_phi = x_54;
  if (x_54) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_58 = asint(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const float x_60 = v1[x_58];
    const float x_62 = asfloat(x_6[2].x);
    const float x_64 = asfloat(x_6[3].x);
    x_69 = (abs((x_60 - (x_62 / x_64))) < E);
    x_70_phi = x_69;
  }
  const bool x_70 = x_70_phi;
  x_86_phi = x_70;
  if (x_70) {
    const int x_74 = asint(x_10[3].x);
    const float x_76 = v1[x_74];
    const float x_78 = asfloat(x_6[2].x);
    const float x_80 = asfloat(x_6[3].x);
    x_85 = (abs((x_76 - (x_78 / x_80))) < E);
    x_86_phi = x_85;
  }
  const bool x_86 = x_86_phi;
  x_102_phi = x_86;
  if (x_86) {
    const int x_90 = asint(x_10[2].x);
    const float x_92 = v1[x_90];
    const float x_94 = asfloat(x_6[2].x);
    const float x_96 = asfloat(x_6[3].x);
    x_101 = (abs((x_92 - (x_94 / x_96))) < E);
    x_102_phi = x_101;
  }
  if (x_102_phi) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_107 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_110 = asint(x_10[1].x);
    const int x_113 = asint(x_10[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_116 = asint(x_10[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_107), float(x_110), float(x_113), float(x_116));
  } else {
    const int x_120 = asint(x_10[1].x);
    const float x_121 = float(x_120);
    x_GLF_color = float4(x_121, x_121, x_121, x_121);
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
