cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[5];
};
cbuffer cbuffer_x_10 : register(b1, space0) {
  uint4 x_10[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2x2 M1 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float a = 0.0f;
  int c = 0;
  const float x_41 = asfloat(x_6[1].x);
  const float x_43 = asfloat(x_6[2].x);
  const float x_45 = asfloat(x_6[3].x);
  const float x_47 = asfloat(x_6[4].x);
  M1 = float2x2(float2(x_41, x_43), float2(x_45, x_47));
  const float x_52 = asfloat(x_6[1].x);
  a = x_52;
  const int x_54 = asint(x_10[1].x);
  c = x_54;
  while (true) {
    const int x_59 = c;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_61 = asint(x_10[scalar_offset / 4][scalar_offset % 4]);
    if ((x_59 < x_61)) {
    } else {
      break;
    }
    const int x_65 = asint(x_10[2].x);
    const float x_70 = M1[x_65][clamp(~(c), 0, 1)];
    a = (a + x_70);
    {
      c = (c + 1);
    }
  }
  const float x_75 = a;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_77 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_75 == x_77)) {
    const int x_83 = asint(x_10[2].x);
    const int x_86 = asint(x_10[1].x);
    const int x_89 = asint(x_10[1].x);
    const int x_92 = asint(x_10[2].x);
    x_GLF_color = float4(float(x_83), float(x_86), float(x_89), float(x_92));
  } else {
    const int x_96 = asint(x_10[2].x);
    const float x_97 = float(x_96);
    x_GLF_color = float4(x_97, x_97, x_97, x_97);
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
