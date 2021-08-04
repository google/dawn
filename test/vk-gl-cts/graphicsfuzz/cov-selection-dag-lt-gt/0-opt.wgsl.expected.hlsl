cbuffer cbuffer_x_5 : register(b1, space0) {
  uint4 x_5[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[2];
};

void main_1() {
  const float x_29 = asfloat(x_5[0].x);
  const float x_31 = asfloat(x_5[0].y);
  if ((x_29 < x_31)) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_37 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
    const int x_40 = asint(x_7[1].x);
    const int x_43 = asint(x_7[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_46 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_GLF_color = float4(float(x_37), float(x_40), float(x_43), float(x_46));
    const float x_50 = asfloat(x_5[0].x);
    const float x_52 = asfloat(x_5[0].y);
    if ((x_50 > x_52)) {
      const uint scalar_offset_2 = ((16u * uint(0))) / 4;
      const int x_57 = asint(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
      const float x_58 = float(x_57);
      x_GLF_color = float4(x_58, x_58, x_58, x_58);
    }
    return;
  } else {
    const int x_61 = asint(x_7[1].x);
    const float x_62 = float(x_61);
    x_GLF_color = float4(x_62, x_62, x_62, x_62);
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
