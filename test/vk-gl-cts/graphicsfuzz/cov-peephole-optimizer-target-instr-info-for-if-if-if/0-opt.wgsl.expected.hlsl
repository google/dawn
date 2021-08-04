static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[2];
};

void main_1() {
  int i = 0;
  const int x_37 = asint(x_6[1].x);
  const float x_38 = float(x_37);
  x_GLF_color = float4(x_38, x_38, x_38, x_38);
  const int x_41 = asint(x_6[1].x);
  i = x_41;
  while (true) {
    const int x_46 = i;
    const int x_48 = asint(x_6[2].x);
    if ((x_46 < x_48)) {
    } else {
      break;
    }
    const float x_52 = gl_FragCoord.y;
    const uint scalar_offset = ((16u * uint(0))) / 4;
    const float x_54 = asfloat(x_9[scalar_offset / 4][scalar_offset % 4]);
    if ((x_52 < x_54)) {
      const float x_59 = gl_FragCoord.x;
      const uint scalar_offset_1 = ((16u * uint(0))) / 4;
      const float x_61 = asfloat(x_9[scalar_offset_1 / 4][scalar_offset_1 % 4]);
      if ((x_59 < x_61)) {
        return;
      }
      const float x_66 = asfloat(x_9[1].x);
      const float x_68 = asfloat(x_9[1].x);
      if ((x_66 > x_68)) {
        return;
      }
      discard;
    }
    const float x_73 = asfloat(x_9[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const float x_75 = asfloat(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if ((x_73 > x_75)) {
      const uint scalar_offset_3 = ((16u * uint(0))) / 4;
      const int x_80 = asint(x_6[scalar_offset_3 / 4][scalar_offset_3 % 4]);
      const int x_83 = asint(x_6[1].x);
      const int x_86 = asint(x_6[1].x);
      const uint scalar_offset_4 = ((16u * uint(0))) / 4;
      const int x_89 = asint(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
      x_GLF_color = float4(float(x_80), float(x_83), float(x_86), float(x_89));
      break;
    }
    const uint scalar_offset_5 = ((16u * uint(0))) / 4;
    const float x_93 = asfloat(x_9[scalar_offset_5 / 4][scalar_offset_5 % 4]);
    if ((x_93 < 0.0f)) {
      discard;
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
