cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_8 : register(b2, space0) {
  uint4 x_8[1];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  bool x_48 = false;
  bool x_49_phi = false;
  const float x_33 = asfloat(x_6[1].x);
  f = pow(-(x_33), sinh(1.0f));
  const float x_37 = f;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_39 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const bool x_40 = (x_37 == x_39);
  x_49_phi = x_40;
  if (!(x_40)) {
    const int x_45 = asint(x_8[0].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_47 = asint(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_48 = (x_45 == x_47);
    x_49_phi = x_48;
  }
  if (x_49_phi) {
    const int x_54 = asint(x_10[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_57 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_60 = asint(x_10[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_63 = asint(x_10[1].x);
    x_GLF_color = float4(float(x_54), float(x_57), float(x_60), float(x_63));
  } else {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_67 = asint(x_10[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const float x_68 = float(x_67);
    x_GLF_color = float4(x_68, x_68, x_68, x_68);
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
