cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2x2 m0 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 m1 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2 v = float2(0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_35 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_37 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  m0 = float2x2(float2(x_35, -0.540302277f), float2(0.540302277f, x_37));
  m1 = mul(m0, m0);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_45 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  v = mul(m1, float2(x_45, x_45));
  const float x_50 = v.x;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const float x_52 = asfloat(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  if ((x_50 < x_52)) {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const float x_58 = asfloat(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const float x_60 = asfloat(x_6[1].x);
    const float x_62 = asfloat(x_6[1].x);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const float x_64 = asfloat(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    x_GLF_color = float4(x_58, x_60, x_62, x_64);
  } else {
    const float x_67 = asfloat(x_6[1].x);
    x_GLF_color = float4(x_67, x_67, x_67, x_67);
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
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
