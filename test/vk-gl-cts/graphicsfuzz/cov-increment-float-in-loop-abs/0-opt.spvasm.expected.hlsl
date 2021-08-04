cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f0 = 0.0f;
  float f1 = 0.0f;
  int i = 0;
  bool x_63 = false;
  bool x_64_phi = false;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_34 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  f0 = x_34;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_36 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  f1 = x_36;
  const int x_38 = asint(x_10[1].x);
  i = x_38;
  while (true) {
    const int x_43 = i;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_45 = asint(x_10[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if ((x_43 < x_45)) {
    } else {
      break;
    }
    f0 = abs((1.100000024f * f0));
    f1 = f0;
    {
      i = (i + 1);
    }
  }
  const float x_54 = f1;
  const float x_56 = asfloat(x_6[1].x);
  const bool x_57 = (x_54 > x_56);
  x_64_phi = x_57;
  if (x_57) {
    const float x_60 = f1;
    const float x_62 = asfloat(x_6[2].x);
    x_63 = (x_60 < x_62);
    x_64_phi = x_63;
  }
  if (x_64_phi) {
    const int x_69 = asint(x_10[2].x);
    const int x_72 = asint(x_10[1].x);
    const int x_75 = asint(x_10[1].x);
    const int x_78 = asint(x_10[2].x);
    x_GLF_color = float4(float(x_69), float(x_72), float(x_75), float(x_78));
  } else {
    const int x_82 = asint(x_10[1].x);
    const float x_83 = float(x_82);
    x_GLF_color = float4(x_83, x_83, x_83, x_83);
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
