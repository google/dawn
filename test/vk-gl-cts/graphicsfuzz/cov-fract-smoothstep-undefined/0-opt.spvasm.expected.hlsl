cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 v1 = float2(0.0f, 0.0f);
  float2 b = float2(0.0f, 0.0f);
  float a = 0.0f;
  bool x_51 = false;
  bool x_52_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_30 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  v1 = float2(x_30, x_30);
  b = frac(v1);
  a = smoothstep(float2(1.0f, 1.0f), float2(1.0f, 1.0f), b).x;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_38 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const float x_39 = a;
  const float x_40 = a;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_42 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  x_GLF_color = float4(x_38, x_39, x_40, x_42);
  const float x_45 = b.x;
  const bool x_46 = (x_45 < 1.0f);
  x_52_phi = x_46;
  if (x_46) {
    const float x_50 = b.y;
    x_51 = (x_50 < 1.0f);
    x_52_phi = x_51;
  }
  if (x_52_phi) {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const float x_57 = asfloat(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_59 = b.x;
    const float x_61 = b.y;
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const float x_63 = asfloat(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    x_GLF_color = float4(x_57, x_59, x_61, x_63);
  } else {
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const float x_66 = asfloat(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    x_GLF_color = float4(x_66, x_66, x_66, x_66);
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
