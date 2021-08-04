cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2x2 m24 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float a = 0.0f;
  float2 v2 = float2(0.0f, 0.0f);
  float2 v3 = float2(0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_40 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_42 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const float x_44 = asfloat(x_8[0].x);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_47 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  m24 = float2x2(float2(x_40, x_42), float2((x_44 * 1.0f), x_47));
  a = m24[0u].x;
  v2 = float2(asfloat(0x7fc00000u), 1.0f);
  const float2 x_53 = v2;
  const float2 x_55 = float2(a, 1.0f);
  v3 = reflect(x_53, x_55);
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const float x_58 = asfloat(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const float2 x_59 = v3;
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const float x_61 = asfloat(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  x_GLF_color = float4(x_58, x_59.x, x_59.y, x_61);
  const float x_66 = asfloat(x_8[0].y);
  const uint scalar_offset_5 = ((16u * uint(0))) / 4;
  const float x_68 = asfloat(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
  if ((x_66 == x_68)) {
    const float4 x_73 = x_GLF_color;
    x_GLF_color = float4(x_73.x, float2(0.0f, 0.0f).x, float2(0.0f, 0.0f).y, x_73.w);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
