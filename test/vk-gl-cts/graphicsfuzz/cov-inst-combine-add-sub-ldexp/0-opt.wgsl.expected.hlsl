cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int i = 0;
  float b = 0.0f;
  const int x_34 = asint(x_6[1].x);
  a = x_34;
  a = (a + 1);
  const int x_38 = asint(x_6[1].x);
  i = x_38;
  while (true) {
    const int x_43 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_45 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    if ((x_43 < x_45)) {
    } else {
      break;
    }
    b = ldexp(float(i), -(a));
    {
      i = (i + 1);
    }
  }
  const float x_55 = b;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_57 = asfloat(x_10[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_55 == x_57)) {
    const int x_63 = asint(x_6[2].x);
    const int x_66 = asint(x_6[1].x);
    const int x_69 = asint(x_6[1].x);
    const int x_72 = asint(x_6[2].x);
    x_GLF_color = float4(float(x_63), float(x_66), float(x_69), float(x_72));
  } else {
    const float x_75 = b;
    x_GLF_color = float4(x_75, x_75, x_75, x_75);
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
