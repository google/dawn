cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float data[2] = (float[2])0;
  float a = 0.0f;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_33 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_35 = asfloat(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  data[x_33] = x_35;
  const int x_38 = asint(x_6[1].x);
  const float x_40 = asfloat(x_8[1].x);
  data[x_38] = x_40;
  const int x_43 = asint(x_6[1].x);
  const float x_47 = data[(1 ^ (x_43 & 2))];
  a = x_47;
  const float x_48 = a;
  const float x_50 = asfloat(x_8[1].x);
  if ((x_48 == x_50)) {
    const float x_56 = asfloat(x_8[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_58 = asfloat(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const float x_60 = asfloat(x_8[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const float x_62 = asfloat(x_8[1].x);
    x_GLF_color = float4(x_56, x_58, x_60, x_62);
  } else {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const float x_65 = asfloat(x_8[scalar_offset_4 / 4][scalar_offset_4 % 4]);
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
