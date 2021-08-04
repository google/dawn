cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float x_23 = asfloat(x_5[1].x);
  if ((rsqrt(x_23) < -1.0f)) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const float x_30 = asfloat(x_5[scalar_offset / 4][scalar_offset % 4]);
    x_GLF_color = float4(x_30, x_30, x_30, x_30);
  } else {
    const float x_33 = asfloat(x_5[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_35 = asfloat(x_5[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_37 = asfloat(x_5[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const float x_39 = asfloat(x_5[1].x);
    x_GLF_color = float4(x_33, x_35, x_37, x_39);
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
