cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[2];
};

void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  a = 1.0f;
  const float x_33 = asfloat(x_7[1].x);
  b = clamp(x_33, (false ? a : 0.0f), 1.0f);
  const float x_37 = b;
  const float x_39 = asfloat(x_7[1].x);
  if ((x_37 == x_39)) {
    const float x_44 = b;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const float x_46 = asfloat(x_7[scalar_offset / 4][scalar_offset % 4]);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_49 = asint(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_52 = asint(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_55 = asint(x_9[1].x);
    x_GLF_color = float4((x_44 * x_46), float(x_49), float(x_52), float(x_55));
  } else {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_59 = asint(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_60 = float(x_59);
    x_GLF_color = float4(x_60, x_60, x_60, x_60);
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
