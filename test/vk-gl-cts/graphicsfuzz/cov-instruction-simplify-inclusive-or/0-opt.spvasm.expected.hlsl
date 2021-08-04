cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float a = 0.0f;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_31 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const int x_34 = asint(x_6[1].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_38 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  a = float2(float(x_31), float(x_34))[(x_38 | 1)];
  const float x_41 = a;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_43 = asfloat(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  if ((x_41 == x_43)) {
    const float x_48 = a;
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_50 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_53 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    x_GLF_color = float4(x_48, float(x_50), float(x_53), a);
  } else {
    const float x_57 = a;
    x_GLF_color = float4(x_57, x_57, x_57, x_57);
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
