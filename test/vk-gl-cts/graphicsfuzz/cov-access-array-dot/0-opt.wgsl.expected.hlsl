cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[3];
};
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int numbers[3] = (int[3])0;
  float2 a = float2(0.0f, 0.0f);
  float b = 0.0f;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_38 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_40 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  numbers[x_38] = x_40;
  const int x_43 = asint(x_6[1].x);
  const int x_45 = asint(x_6[1].x);
  numbers[x_43] = x_45;
  const int x_48 = asint(x_6[2].x);
  const int x_50 = asint(x_6[2].x);
  numbers[x_48] = x_50;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_53 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const float x_56 = asfloat(x_9[2].x);
  const int x_60 = numbers[((0.0f < x_56) ? 1 : 2)];
  a = float2(float(x_53), float(x_60));
  const float2 x_63 = a;
  const float x_65 = asfloat(x_9[1].x);
  const float x_67 = asfloat(x_9[1].x);
  b = dot(x_63, float2(x_65, x_67));
  const float x_70 = b;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const float x_72 = asfloat(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  if ((x_70 == x_72)) {
    const int x_78 = asint(x_6[1].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_81 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_84 = asint(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const int x_87 = asint(x_6[1].x);
    x_GLF_color = float4(float(x_78), float(x_81), float(x_84), float(x_87));
  } else {
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_91 = asint(x_6[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const float x_92 = float(x_91);
    x_GLF_color = float4(x_92, x_92, x_92, x_92);
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
