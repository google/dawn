float4 tint_unpack4x8unorm(uint param_0) {
  uint j = param_0;
  uint4 i = uint4(j & 0xff, (j >> 8) & 0xff, (j >> 16) & 0xff, j >> 24);
  return float4(i) / 255.0;
}

cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[2];
};
cbuffer cbuffer_x_10 : register(b2, space0) {
  uint4 x_10[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const uint x_39 = x_6[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const uint x_41 = x_6[scalar_offset_1 / 4][scalar_offset_1 % 4];
  v = tint_unpack4x8unorm((x_39 / (true ? 92382u : x_41)));
  const float4 x_45 = v;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_47 = asint(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_50 = asint(x_8[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const int x_53 = asint(x_8[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  const float x_56 = asfloat(x_10[1].x);
  const float x_58 = asfloat(x_10[2].x);
  const uint scalar_offset_5 = ((16u * uint(0))) / 4;
  const float x_63 = asfloat(x_10[scalar_offset_5 / 4][scalar_offset_5 % 4]);
  if ((distance(x_45, float4(float(x_47), float(x_50), float(x_53), (x_56 / x_58))) < x_63)) {
    const int x_69 = asint(x_8[1].x);
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_72 = asint(x_8[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const uint scalar_offset_7 = ((16u * uint(0))) / 4;
    const int x_75 = asint(x_8[scalar_offset_7 / 4][scalar_offset_7 % 4]);
    const int x_78 = asint(x_8[1].x);
    x_GLF_color = float4(float(x_69), float(x_72), float(x_75), float(x_78));
  } else {
    const uint scalar_offset_8 = ((16u * uint(0))) / 4;
    const int x_82 = asint(x_8[scalar_offset_8 / 4][scalar_offset_8 % 4]);
    const float x_83 = float(x_82);
    x_GLF_color = float4(x_83, x_83, x_83, x_83);
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
