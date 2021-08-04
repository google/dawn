cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[5];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int i = 0;
  int indexable[9] = (int[9])0;
  const int x_38 = asint(x_6[2].x);
  a = x_38;
  const int x_40 = asint(x_6[3].x);
  i = x_40;
  while (true) {
    const int x_45 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_47 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    if ((x_45 < x_47)) {
    } else {
      break;
    }
    const int x_50 = i;
    const int x_52 = asint(x_6[4].x);
    const int tint_symbol_2[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    indexable = tint_symbol_2;
    const int x_55 = indexable[(x_50 % x_52)];
    a = (a + x_55);
    {
      i = (i + 1);
    }
  }
  const int x_60 = a;
  const int x_62 = asint(x_6[1].x);
  if ((x_60 == x_62)) {
    const int x_68 = asint(x_6[2].x);
    const int x_71 = asint(x_6[3].x);
    const int x_74 = asint(x_6[3].x);
    const int x_77 = asint(x_6[2].x);
    x_GLF_color = float4(float(x_68), float(x_71), float(x_74), float(x_77));
  } else {
    const int x_81 = asint(x_6[3].x);
    const float x_82 = float(x_81);
    x_GLF_color = float4(x_82, x_82, x_82, x_82);
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
