cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[2];
};

void main_1() {
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_29 = asfloat(x_5[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_32 = asfloat(x_5[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((ldexp(x_29, 10000) == x_32)) {
    const int x_38 = asint(x_7[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_41 = asint(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_44 = asint(x_7[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_47 = asint(x_7[1].x);
    x_GLF_color = float4(float(x_38), float(x_41), float(x_44), float(x_47));
  } else {
    const int x_51 = asint(x_7[1].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_54 = asint(x_7[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_57 = asint(x_7[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const int x_60 = asint(x_7[1].x);
    x_GLF_color = float4(float(x_51), float(x_54), float(x_57), float(x_60));
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
