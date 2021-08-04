cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[2];
};
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[2];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float arr[5] = (float[5])0;
  int i = 0;
  int j = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_38 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const float x_40 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const float x_42 = asfloat(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const float x_44 = asfloat(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  const uint scalar_offset_4 = ((16u * uint(0))) / 4;
  const float x_46 = asfloat(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
  const float tint_symbol_3[5] = {x_38, x_40, x_42, x_44, x_46};
  arr = tint_symbol_3;
  const int x_49 = asint(x_9[1].x);
  i = x_49;
  j = 0;
  while (true) {
    const int x_54 = i;
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_56 = asint(x_9[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    if ((x_54 < x_56)) {
    } else {
      break;
    }
    if ((j < -1)) {
      break;
    }
    const int x_63 = j;
    const float x_65 = arr[x_63];
    arr[x_63] = (x_65 + 1.0f);
    {
      i = (i + 1);
      j = (j + 1);
    }
  }
  const uint scalar_offset_6 = ((16u * uint(0))) / 4;
  const float x_73 = asfloat(x_6[scalar_offset_6 / 4][scalar_offset_6 % 4]);
  const float x_75 = asfloat(x_6[1].x);
  const float x_77 = asfloat(x_6[1].x);
  const uint scalar_offset_7 = ((16u * uint(0))) / 4;
  const float x_79 = asfloat(x_6[scalar_offset_7 / 4][scalar_offset_7 % 4]);
  x_GLF_color = float4(x_73, x_75, x_77, x_79);
  const int x_82 = asint(x_9[1].x);
  i = x_82;
  while (true) {
    const int x_87 = i;
    const uint scalar_offset_8 = ((16u * uint(0))) / 4;
    const int x_89 = asint(x_9[scalar_offset_8 / 4][scalar_offset_8 % 4]);
    if ((x_87 < x_89)) {
    } else {
      break;
    }
    const float x_94 = arr[i];
    if (!((x_94 == 2.0f))) {
      const float x_99 = asfloat(x_6[1].x);
      x_GLF_color = float4(x_99, x_99, x_99, x_99);
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
