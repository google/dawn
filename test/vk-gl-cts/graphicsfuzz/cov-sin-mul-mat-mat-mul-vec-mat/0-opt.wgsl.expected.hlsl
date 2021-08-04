cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[2];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int f1_vf2_(inout float2 v1) {
  bool x_99 = false;
  bool x_100_phi = false;
  const float x_89 = v1.x;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_91 = asfloat(x_7[scalar_offset / 4][scalar_offset % 4]);
  const bool x_92 = (x_89 == x_91);
  x_100_phi = x_92;
  if (x_92) {
    const float x_96 = v1.y;
    const float x_98 = asfloat(x_7[1].x);
    x_99 = (x_96 == x_98);
    x_100_phi = x_99;
  }
  if (x_100_phi) {
    const int x_104 = asint(x_9[1].x);
    return x_104;
  }
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_106 = asint(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  return x_106;
}

void main_1() {
  float2x2 m1 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 m2 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2 v1_1 = float2(0.0f, 0.0f);
  int a = 0;
  float2 param = float2(0.0f, 0.0f);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_45 = asfloat(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const float x_47 = asfloat(x_7[1].x);
  const float x_50 = asfloat(x_7[1].x);
  const float x_52 = asfloat(x_7[1].x);
  m1 = float2x2(float2(x_45, -(x_47)), float2(x_50, sin(x_52)));
  m2 = mul(m1, m1);
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const float x_61 = asfloat(x_7[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  v1_1 = mul(m2, float2(x_61, x_61));
  param = v1_1;
  const int x_66 = f1_vf2_(param);
  a = x_66;
  const int x_67 = a;
  const int x_69 = asint(x_9[1].x);
  if ((x_67 == x_69)) {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const float x_75 = asfloat(x_7[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const float x_77 = asfloat(x_7[1].x);
    const float x_79 = asfloat(x_7[1].x);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const float x_81 = asfloat(x_7[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    x_GLF_color = float4(x_75, x_77, x_79, x_81);
  } else {
    const int x_84 = asint(x_9[1].x);
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
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
