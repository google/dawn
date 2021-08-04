cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[5];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int func_i1_(inout int x) {
  int a = 0;
  int i = 0;
  int indexable[4] = (int[4])0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_72 = asint(x_8[scalar_offset / 4][scalar_offset % 4]);
  a = x_72;
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_74 = asint(x_8[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  i = x_74;
  while (true) {
    const int x_79 = i;
    const int x_81 = asint(x_8[1].x);
    if ((x_79 < x_81)) {
    } else {
      break;
    }
    const int x_85 = asint(x_8[3].x);
    const int x_87 = asint(x_8[3].x);
    const int x_89 = asint(x_8[3].x);
    const int x_91 = asint(x_8[3].x);
    const int x_93 = a;
    const int tint_symbol_2[4] = {x_85, x_87, x_89, x_91};
    indexable = tint_symbol_2;
    const int x_95 = indexable[x_93];
    const int x_96 = x;
    if ((x_95 > x_96)) {
      if (true) {
        const int x_105 = asint(x_8[3].x);
        return x_105;
      } else {
        const int x_107 = asint(x_8[3].x);
        a = x_107;
      }
    } else {
      if (true) {
        const int x_111 = asint(x_8[4].x);
        return x_111;
      }
    }
    {
      i = (i + 1);
    }
  }
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_115 = asint(x_8[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  return x_115;
}

void main_1() {
  int a_1 = 0;
  int param = 0;
  int param_1 = 0;
  const uint scalar_offset_3 = ((16u * uint(0))) / 4;
  const int x_39 = asint(x_8[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  param = x_39;
  const int x_40 = func_i1_(param);
  a_1 = x_40;
  const int x_42 = asint(x_8[3].x);
  param_1 = x_42;
  const int x_43 = func_i1_(param_1);
  a_1 = (a_1 + x_43);
  const int x_46 = a_1;
  const int x_48 = asint(x_8[2].x);
  if ((x_46 == x_48)) {
    const int x_54 = asint(x_8[3].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const int x_57 = asint(x_8[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const int x_60 = asint(x_8[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    const int x_63 = asint(x_8[3].x);
    x_GLF_color = float4(float(x_54), float(x_57), float(x_60), float(x_63));
  } else {
    const uint scalar_offset_6 = ((16u * uint(0))) / 4;
    const int x_67 = asint(x_8[scalar_offset_6 / 4][scalar_offset_6 % 4]);
    const float x_68 = float(x_67);
    x_GLF_color = float4(x_68, x_68, x_68, x_68);
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
