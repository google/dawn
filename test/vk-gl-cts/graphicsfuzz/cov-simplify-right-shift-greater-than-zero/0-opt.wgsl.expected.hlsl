cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_22 = asint(x_5[scalar_offset / 4][scalar_offset % 4]);
  if (((1 >> asuint(x_22)) > 0)) {
    const int x_29 = asint(x_5[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_32 = asint(x_5[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_35 = asint(x_5[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_38 = asint(x_5[1].x);
    x_GLF_color = float4(float(x_29), float(x_32), float(x_35), float(x_38));
  } else {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_42 = asint(x_5[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_43 = float(x_42);
    x_GLF_color = float4(x_43, x_43, x_43, x_43);
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
