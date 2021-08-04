cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[2];
};

void main_1() {
  bool b = false;
  b = true;
  const float x_38 = asfloat(x_6[0].x);
  const float x_40 = asfloat(x_6[0].y);
  if ((x_38 > x_40)) {
    const float x_45 = asfloat(x_6[0].x);
    const float x_47 = asfloat(x_6[0].y);
    if ((x_45 < x_47)) {
      b = false;
    }
  }
  if (b) {
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_10 = asint(x_8[scalar_offset / 4][scalar_offset % 4]);
    const int x_11 = asint(x_8[1].x);
    const int x_12 = asint(x_8[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_13 = asint(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    x_GLF_color = float4(float(x_10), float(x_11), float(x_12), float(x_13));
  } else {
    const int x_14 = asint(x_8[1].x);
    const float x_65 = float(x_14);
    x_GLF_color = float4(x_65, x_65, x_65, x_65);
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
