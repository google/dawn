cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[2];
};

void main_1() {
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_31 = asfloat(x_5[scalar_offset / 4][scalar_offset % 4]);
  if ((sqrt(x_31) < -1.0f)) {
    const int x_10 = asint(x_7[1].x);
    const float x_38 = float(x_10);
    x_GLF_color = float4(x_38, x_38, x_38, x_38);
  } else {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_11 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const float x_41 = float(x_11);
    const int x_12 = asint(x_7[1].x);
    const float x_43 = float(x_12);
    x_GLF_color = float4(x_41, x_43, x_43, x_41);
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
