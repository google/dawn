cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};

void main_1() {
  int i = 0;
  const int x_32 = asint(x_6[2].x);
  i = x_32;
  while (true) {
    if ((i >= 0)) {
    } else {
      break;
    }
    if (((i % 2) == 0)) {
      const uint scalar_offset = ((16u * uint(0))) / 4;
      const int x_47 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
      const uint scalar_offset_1 = ((16u * uint(0))) / 4;
      const int x_50 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
      const int x_53 = asint(x_6[1].x);
      x_GLF_color = float4(1.0f, float(x_47), float(x_50), float(x_53));
    } else {
      const uint scalar_offset_2 = ((16u * uint(0))) / 4;
      const float x_57 = asfloat(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
      x_GLF_color = float4(x_57, x_57, x_57, x_57);
    }
    i = (i - 1);
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
