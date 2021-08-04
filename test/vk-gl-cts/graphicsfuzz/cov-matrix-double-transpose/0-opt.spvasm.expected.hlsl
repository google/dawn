cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2x2 m = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_29 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const float x_30 = float(x_29);
  m = transpose(transpose(float2x2(float2(x_30, 0.0f), float2(0.0f, x_30))));
  const float2x2 x_36 = m;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_38 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const float x_39 = float(x_38);
  const float2x2 x_42 = float2x2(float2(x_39, 0.0f), float2(0.0f, x_39));
  if ((all((x_36[0u] == x_42[0u])) & all((x_36[1u] == x_42[1u])))) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_56 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_59 = asint(x_6[1].x);
    const int x_62 = asint(x_6[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_65 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_56), float(x_59), float(x_62), float(x_65));
  } else {
    const int x_69 = asint(x_6[1].x);
    const float x_70 = float(x_69);
    x_GLF_color = float4(x_70, x_70, x_70, x_70);
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
