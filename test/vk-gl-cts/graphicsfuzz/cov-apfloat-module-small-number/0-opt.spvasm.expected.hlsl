static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[2];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};

void main_1() {
  float f0 = 0.0f;
  float s1 = 0.0f;
  float f1 = 0.0f;
  bool x_72 = false;
  bool x_73_phi = false;
  f0 = (10.0f - (0.000001f * floor((10.0f / 0.000001f))));
  s1 = 9.99999935e-39f;
  if ((s1 == 0.0f)) {
    s1 = 1.0f;
  }
  bool x_62 = false;
  bool x_71 = false;
  bool x_63_phi = false;
  bool x_72_phi = false;
  const float x_42 = s1;
  f1 = (10.0f - (x_42 * floor((10.0f / x_42))));
  const bool x_48 = (isinf(f1) | (s1 == 1.0f));
  x_73_phi = x_48;
  if (!(x_48)) {
    const bool x_54 = (f0 == f1);
    x_63_phi = x_54;
    if (!(x_54)) {
      x_62 = ((f0 > 0.99000001f) & (f0 < 0.01f));
      x_63_phi = x_62;
    }
    const bool x_63 = x_63_phi;
    x_72_phi = x_63;
    if (!(x_63)) {
      x_71 = ((f1 > 0.99000001f) & (f1 < 0.01f));
      x_72_phi = x_71;
    }
    x_72 = x_72_phi;
    x_73_phi = x_72;
  }
  if ((x_73_phi | (f1 == 10.0f))) {
    const int x_81 = asint(x_8[1].x);
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_84 = asint(x_8[scalar_offset / 4][scalar_offset % 4]);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_87 = asint(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_90 = asint(x_8[1].x);
    x_GLF_color = float4(float(x_81), float(x_84), float(x_87), float(x_90));
  } else {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_94 = asint(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const float x_95 = float(x_94);
    x_GLF_color = float4(x_95, x_95, x_95, x_95);
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
