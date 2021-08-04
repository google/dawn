cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[2];
};

void main_1() {
  float2 v0 = float2(0.0f, 0.0f);
  float2 v1 = float2(0.0f, 0.0f);
  const float x_36 = asfloat(x_6[1].x);
  v0 = float2(x_36, -580.015014648f);
  const float2 x_38 = v0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_41 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const float x_42 = -(x_41);
  v1 = clamp(ceil(x_38), float2(x_42, x_42), float2(100.0f, 100.0f));
  const float x_46 = v1.y;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_48 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_46 == -(x_48))) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_55 = asint(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_58 = asint(x_9[1].x);
    const int x_61 = asint(x_9[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_64 = asint(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_55), float(x_58), float(x_61), float(x_64));
  } else {
    const int x_68 = asint(x_9[1].x);
    const float x_69 = float(x_68);
    x_GLF_color = float4(x_69, x_69, x_69, x_69);
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
