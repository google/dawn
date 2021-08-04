static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[6];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_12 : register(b0, space0) {
  uint4 x_12[1];
};

void main_1() {
  int data[5] = (int[5])0;
  int a = 0;
  int i = 0;
  int j = 0;
  int i_1 = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_45 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const int x_48 = asint(x_6[5].x);
  const int x_51 = asint(x_6[5].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_54 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  x_GLF_color = float4(float(x_45), float(x_48), float(x_51), float(x_54));
  const int x_58 = asint(x_6[1].x);
  const int x_60 = asint(x_6[2].x);
  const int x_62 = asint(x_6[3].x);
  const int x_64 = asint(x_6[4].x);
  const uint scalar_offset_2 = ((16u * uint(0))) / 4;
  const int x_66 = asint(x_6[scalar_offset_2 / 4][scalar_offset_2 % 4]);
  const int tint_symbol_5[5] = {x_58, x_60, x_62, x_64, x_66};
  data = tint_symbol_5;
  const int x_69 = asint(x_6[5].x);
  a = x_69;
  while (true) {
    const int x_74 = a;
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_76 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    if ((x_74 < x_76)) {
    } else {
      break;
    }
    const int x_80 = asint(x_6[5].x);
    i = x_80;
    while (true) {
      const int x_85 = i;
      const int x_87 = asint(x_6[1].x);
      if ((x_85 < x_87)) {
      } else {
        break;
      }
      j = i;
      while (true) {
        const int x_95 = j;
        const int x_97 = asint(x_6[1].x);
        if ((x_95 < x_97)) {
        } else {
          break;
        }
        const int x_102 = data[i];
        const int x_105 = data[j];
        if ((x_102 < x_105)) {
          const int x_110 = asint(x_6[5].x);
          const float x_111 = float(x_110);
          x_GLF_color = float4(x_111, x_111, x_111, x_111);
        }
        {
          j = (j + 1);
        }
      }
      {
        i = (i + 1);
      }
    }
    {
      a = (a + 1);
    }
  }
  while (true) {
    const float x_124 = gl_FragCoord.x;
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const float x_126 = asfloat(x_12[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    if ((x_124 < x_126)) {
    } else {
      break;
    }
    const int x_130 = asint(x_6[5].x);
    i_1 = x_130;
    while (true) {
      const int x_135 = i_1;
      const uint scalar_offset_5 = ((16u * uint(0))) / 4;
      const int x_137 = asint(x_6[scalar_offset_5 / 4][scalar_offset_5 % 4]);
      if ((x_135 < x_137)) {
      } else {
        break;
      }
      const int x_141 = asint(x_6[5].x);
      const float x_142 = float(x_141);
      x_GLF_color = float4(x_142, x_142, x_142, x_142);
      {
        i_1 = (i_1 + 1);
      }
    }
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_6 = {x_GLF_color};
  return tint_symbol_6;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
