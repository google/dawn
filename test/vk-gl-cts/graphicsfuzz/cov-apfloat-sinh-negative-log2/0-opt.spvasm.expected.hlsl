cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float undefined = 0.0f;
  bool x_45 = false;
  bool x_46_phi = false;
  undefined = sinh(asfloat(0x7fc00000u));
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_10 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const bool x_38 = (1 == x_10);
  x_46_phi = x_38;
  if (!(x_38)) {
    const float x_42 = undefined;
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_44 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_45 = (x_42 > x_44);
    x_46_phi = x_45;
  }
  if (x_46_phi) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_12 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_13 = asint(x_6[1].x);
    const int x_14 = asint(x_6[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_15 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_12), float(x_13), float(x_14), float(x_15));
  } else {
    const int x_16 = asint(x_6[1].x);
    const float x_60 = float(x_16);
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
