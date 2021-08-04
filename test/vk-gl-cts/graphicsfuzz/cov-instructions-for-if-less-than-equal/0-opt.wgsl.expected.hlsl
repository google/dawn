static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b1, space0) {
  uint4 x_5[2];
};
cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[2];
};

void main_1() {
  int i = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_29 = asint(x_5[scalar_offset / 4][scalar_offset % 4]);
  const float x_30 = float(x_29);
  x_GLF_color = float4(x_30, x_30, x_30, x_30);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_33 = asint(x_5[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  i = x_33;
  while (true) {
    const int x_38 = i;
    const int x_40 = asint(x_5[1].x);
    if ((x_38 < x_40)) {
    } else {
      break;
    }
    const float x_44 = asfloat(x_8[1].x);
    if (!((x_44 <= float(i)))) {
      const uint scalar_offset_2 = ((16u * uint(0))) / 4;
      const float x_52 = asfloat(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
      const int x_53 = i;
      const int x_55 = i;
      const uint scalar_offset_3 = ((16u * uint(0))) / 4;
      const float x_58 = asfloat(x_8[scalar_offset_3 / 4][scalar_offset_3 % 4]);
      x_GLF_color = (x_GLF_color + float4(x_52, float(x_53), float(x_55), x_58));
    }
    {
      i = (i + 1);
    }
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
