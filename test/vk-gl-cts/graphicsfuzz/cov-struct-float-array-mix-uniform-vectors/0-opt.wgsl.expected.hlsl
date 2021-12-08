SKIP: FAILED

struct S {
  float numbers[3];
};

cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[5];
};
cbuffer cbuffer_x_9 : register(b2, space0) {
  uint4 x_9[1];
};
cbuffer cbuffer_x_12 : register(b3, space0) {
  uint4 x_12[1];
};
cbuffer cbuffer_x_15 : register(b0, space0) {
  uint4 x_15[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  S obj = (S)0;
  float a = 0.0f;
  float2 x_49 = float2(0.0f, 0.0f);
  float b = 0.0f;
  const float x_51 = asfloat(x_7[3].x);
  const float x_53 = asfloat(x_7[2].x);
  const float x_55 = asfloat(x_7[4].x);
  const float tint_symbol_6[3] = {x_51, x_53, x_55};
  const S tint_symbol_7 = {tint_symbol_6};
  obj = tint_symbol_7;
  const float x_59 = asfloat(x_9[0].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_62 = asfloat(x_7[scalar_offset / 4][scalar_offset % 4]);
  obj.numbers[int(x_59)] = x_62;
  const float x_65 = asfloat(x_9[0].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_67 = asfloat(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_65 > x_67)) {
    const float2 x_73 = asfloat(x_9[0].xy);
    x_49 = x_73;
  } else {
    const float2 x_75 = asfloat(x_12[0].xy);
    x_49 = x_75;
  }
  const float x_77 = x_49.y;
  a = x_77;
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_79 = asfloat(x_7[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const float x_80 = a;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_82 = asint(x_15[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const float x_84 = obj.numbers[x_82];
  b = lerp(x_79, x_80, x_84);
  const float x_86 = b;
  const float x_88 = asfloat(x_7[2].x);
  const float x_91 = asfloat(x_7[1].x);
  if ((distance(x_86, x_88) < x_91)) {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_97 = asint(x_15[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const int x_100 = asint(x_15[1].x);
    const int x_103 = asint(x_15[1].x);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_106 = asint(x_15[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    x_GLF_color = float4(float(x_97), float(x_100), float(x_103), float(x_106));
  } else {
    const int x_110 = asint(x_15[1].x);
    const float x_111 = float(x_110);
    x_GLF_color = float4(x_111, x_111, x_111, x_111);
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
  const main_out tint_symbol_8 = {x_GLF_color};
  return tint_symbol_8;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
C:\src\tint\test\Shader@0x00000172CC1515F0(33,3-24): error X3500: array reference cannot be used as an l-value; not natively addressable

