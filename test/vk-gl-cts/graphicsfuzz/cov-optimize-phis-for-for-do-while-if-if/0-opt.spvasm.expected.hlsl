cbuffer cbuffer_x_7 : register(b1, space0) {
  uint4 x_7[3];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int i = 0;
  int j = 0;
  const int x_36 = asint(x_7[2].x);
  a = x_36;
  const int x_38 = asint(x_7[2].x);
  i = x_38;
  while (true) {
    const int x_43 = i;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const int x_45 = asint(x_7[scalar_offset / 4][scalar_offset % 4]);
    if ((x_43 < x_45)) {
    } else {
      break;
    }
    const int x_49 = asint(x_7[2].x);
    j = x_49;
    while (true) {
      const int x_54 = j;
      const uint scalar_offset_1 = ((16u * uint(0))) / 4;
      const int x_56 = asint(x_7[scalar_offset_1 / 4][scalar_offset_1 % 4]);
      if ((x_54 < x_56)) {
      } else {
        break;
      }
      while (true) {
        const int x_64 = asint(x_7[1].x);
        a = x_64;
        const float x_66 = gl_FragCoord.y;
        const uint scalar_offset_2 = ((16u * uint(0))) / 4;
        const float x_68 = asfloat(x_11[scalar_offset_2 / 4][scalar_offset_2 % 4]);
        if ((x_66 < x_68)) {
          discard;
        }
        {
          const int x_72 = a;
          const int x_74 = asint(x_7[1].x);
          if ((x_72 < x_74)) {
          } else {
            break;
          }
        }
      }
      const float x_77 = gl_FragCoord.y;
      const uint scalar_offset_3 = ((16u * uint(0))) / 4;
      const float x_79 = asfloat(x_11[scalar_offset_3 / 4][scalar_offset_3 % 4]);
      if ((x_77 < x_79)) {
        break;
      }
      {
        j = (j + 1);
      }
    }
    {
      i = (i + 1);
    }
  }
  const int x_87 = a;
  const int x_89 = asint(x_7[1].x);
  if ((x_87 == x_89)) {
    const int x_94 = a;
    const int x_97 = asint(x_7[2].x);
    const int x_100 = asint(x_7[2].x);
    x_GLF_color = float4(float(x_94), float(x_97), float(x_100), float(a));
  } else {
    const int x_106 = asint(x_7[2].x);
    const float x_107 = float(x_106);
    x_GLF_color = float4(x_107, x_107, x_107, x_107);
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
  const main_out tint_symbol_5 = {x_GLF_color};
  return tint_symbol_5;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
