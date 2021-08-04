cbuffer cbuffer_x_6 : register(b2, space0) {
  uint4 x_6[1];
};
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};

void main_1() {
  float f = 0.0f;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const uint x_36 = x_6[scalar_offset / 4][scalar_offset % 4];
  f = asfloat(max(100u, x_36));
  const float x_39 = f;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_41 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_39 == x_41)) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_47 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_50 = asint(x_10[1].x);
    const int x_53 = asint(x_10[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_56 = asint(x_10[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_47), float(x_50), float(x_53), float(x_56));
  } else {
    const int x_60 = asint(x_10[1].x);
    const float x_61 = float(x_60);
    x_GLF_color = float4(x_61, x_61, x_61, x_61);
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
