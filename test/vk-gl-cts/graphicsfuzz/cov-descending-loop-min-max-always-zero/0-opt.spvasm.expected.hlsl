cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  int i = 0;
  float a = 0.0f;
  const float x_37 = asfloat(x_6[1].x);
  f = x_37;
  const int x_39 = asint(x_9[1].x);
  i = x_39;
  while (true) {
    const int x_44 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_46 = asint(x_9[scalar_offset / 4][scalar_offset % 4]);
    if ((x_44 > x_46)) {
    } else {
      break;
    }
    a = (1.0f - max(1.0f, float(i)));
    f = min(max(a, 0.0f), 0.0f);
    {
      i = (i - 1);
    }
  }
  const float x_58 = f;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_60 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  if ((x_58 == x_60)) {
    const int x_66 = asint(x_9[2].x);
    const float x_68 = f;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_70 = asint(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_GLF_color = float4(float(x_66), x_68, float(x_70), 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
