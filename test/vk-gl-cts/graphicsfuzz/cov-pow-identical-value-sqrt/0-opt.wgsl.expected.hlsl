cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_11 : register(b2, space0) {
  uint4 x_11[1];
};
cbuffer cbuffer_x_13 : register(b1, space0) {
  uint4 x_13[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f0 = 0.0f;
  float f1 = 0.0f;
  float f2 = 0.0f;
  float f3 = 0.0f;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_36 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  f0 = x_36;
  const float x_38 = asfloat(x_6[1].x);
  f1 = (x_38 * pow(f0, 4.0f));
  const float x_43 = asfloat(x_6[1].x);
  f2 = (x_43 * pow(f0, 4.0f));
  const float x_47 = f1;
  const float x_48 = f2;
  const float x_51 = asfloat(x_11[0].x);
  f3 = sqrt((((x_47 - x_48) - x_51) + f0));
  const float x_56 = f3;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_59 = asint(x_13[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((int(x_56) == x_59)) {
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_65 = asint(x_13[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const int x_68 = asint(x_13[1].x);
    const int x_71 = asint(x_13[1].x);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_74 = asint(x_13[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(float(x_65), float(x_68), float(x_71), float(x_74));
  } else {
    const int x_78 = asint(x_13[1].x);
    const float x_79 = float(x_78);
    x_GLF_color = float4(x_79, x_79, x_79, x_79);
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
