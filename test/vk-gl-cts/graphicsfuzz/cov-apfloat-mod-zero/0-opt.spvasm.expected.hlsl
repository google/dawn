cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float undefined = 0.0f;
  bool x_51 = false;
  bool x_52_phi = false;
  undefined = (5.0f - (0.0f * floor((5.0f / 0.0f))));
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_10 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_11 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const int x_12 = asint(x_6[1].x);
  const bool x_44 = (x_10 == (x_11 + x_12));
  x_52_phi = x_44;
  if (!(x_44)) {
    const float x_48 = undefined;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_50 = asfloat(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_51 = (x_48 > x_50);
    x_52_phi = x_51;
  }
  if (x_52_phi) {
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_15 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_16 = asint(x_6[1].x);
    const int x_17 = asint(x_6[1].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_18 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    x_GLF_color = float4(float(x_15), float(x_16), float(x_17), float(x_18));
  } else {
    const int x_19 = asint(x_6[1].x);
    const float x_66 = float(x_19);
    x_GLF_color = float4(x_66, x_66, x_66, x_66);
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
