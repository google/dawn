cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[4];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[3];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float arr[3] = (float[3])0;
  int i = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_36 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const float x_38 = asfloat(x_6[1].x);
  const float x_40 = asfloat(x_6[2].x);
  const float tint_symbol_3[3] = {x_36, x_38, x_40};
  arr = tint_symbol_3;
  i = 1;
  while (true) {
    const int x_46 = i;
    const int x_48 = asint(x_9[2].x);
    if ((x_46 < min(x_48, 3))) {
    } else {
      break;
    }
    const int x_53 = asint(x_9[2].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_55 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const float x_57 = arr[x_53];
    arr[x_53] = (x_57 + x_55);
    {
      i = (i + 1);
    }
  }
  const int x_63 = asint(x_9[2].x);
  const float x_65 = arr[x_63];
  const float x_67 = asfloat(x_6[3].x);
  if ((x_65 == x_67)) {
    const int x_73 = asint(x_9[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_76 = asint(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_79 = asint(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    const int x_82 = asint(x_9[1].x);
    x_GLF_color = float4(float(x_73), float(x_76), float(x_79), float(x_82));
  } else {
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_86 = asint(x_9[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const float x_87 = float(x_86);
    x_GLF_color = float4(x_87, x_87, x_87, x_87);
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
