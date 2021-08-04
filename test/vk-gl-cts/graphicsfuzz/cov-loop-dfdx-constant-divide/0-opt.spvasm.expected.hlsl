cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_11 : register(b1, space0) {
  uint4 x_11[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float a = 0.0f;
  float b = 0.0f;
  float c = 0.0f;
  int i = 0;
  const float x_35 = asfloat(x_6[1].x);
  a = x_35;
  const float x_37 = asfloat(x_6[1].x);
  b = x_37;
  const float x_39 = asfloat(x_6[1].x);
  c = x_39;
  const int x_41 = asint(x_11[1].x);
  i = x_41;
  while (true) {
    const int x_46 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_48 = asint(x_11[scalar_offset / 4][scalar_offset % 4]);
    if ((x_46 < x_48)) {
    } else {
      break;
    }
    const int x_51 = i;
    const int x_53 = asint(x_11[2].x);
    if ((x_51 == x_53)) {
      const float x_57 = a;
      const float x_60 = asfloat(x_6[1].x);
      b = (ddx(x_57) + x_60);
    }
    c = ddx(a);
    a = (c / b);
    {
      i = (i + 1);
    }
  }
  const float x_69 = a;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_71 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_69 == x_71)) {
    const int x_77 = asint(x_11[2].x);
    const int x_80 = asint(x_11[1].x);
    const int x_83 = asint(x_11[1].x);
    const int x_86 = asint(x_11[2].x);
    x_GLF_color = float4(float(x_77), float(x_80), float(x_83), float(x_86));
  } else {
    const int x_90 = asint(x_11[1].x);
    const float x_91 = float(x_90);
    x_GLF_color = float4(x_91, x_91, x_91, x_91);
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
