cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[5];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int b = 0;
  int i = 0;
  int i_1 = 0;
  int i_2 = 0;
  int indexable[2] = (int[2])0;
  const int x_36 = asint(x_6[2].x);
  a = x_36;
  const int x_38 = asint(x_6[3].x);
  b = x_38;
  const int x_40 = asint(x_6[2].x);
  const float x_41 = float(x_40);
  x_GLF_color = float4(x_41, x_41, x_41, x_41);
  const int x_44 = asint(x_6[2].x);
  i = x_44;
  while (true) {
    const int x_49 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_51 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
    if ((x_49 < x_51)) {
    } else {
      break;
    }
    const int x_54 = i;
    const int x_56 = asint(x_6[3].x);
    if ((x_54 > x_56)) {
      a = (a + 1);
      if (false) {
        const int x_65 = asint(x_6[2].x);
        i_1 = x_65;
        while (true) {
          const int x_70 = i_1;
          const uint scalar_offset_1 = ((16u * uint(0))) / 4;
          const int x_72 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
          if ((x_70 < x_72)) {
          } else {
            break;
          }
          return;
        }
      }
    }
    {
      i = (i + 1);
    }
  }
  const int x_78 = asint(x_6[2].x);
  i_2 = x_78;
  while (true) {
    const int x_83 = i_2;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_85 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if ((x_83 < x_85)) {
    } else {
      break;
    }
    const int x_89 = asint(x_6[3].x);
    const int x_91 = asint(x_6[4].x);
    const int x_93 = b;
    const int tint_symbol_2[2] = {x_89, x_91};
    indexable = tint_symbol_2;
    const int x_95 = indexable[x_93];
    a = (a + x_95);
    {
      i_2 = (i_2 + 1);
    }
  }
  const int x_100 = a;
  const int x_102 = asint(x_6[1].x);
  if ((x_100 == x_102)) {
    const int x_107 = asint(x_6[3].x);
    const int x_110 = asint(x_6[2].x);
    const int x_113 = asint(x_6[2].x);
    const int x_116 = asint(x_6[3].x);
    x_GLF_color = float4(float(x_107), float(x_110), float(x_113), float(x_116));
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
