static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b1, space0) {
  uint4 x_6[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};

void main_1() {
  int2 icoord = int2(0, 0);
  float x_40 = 0.0f;
  int2 icoord_1 = int2(0, 0);
  const float x_42 = gl_FragCoord.x;
  const float x_44 = asfloat(x_6[1].x);
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_47 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  if (((x_42 * x_44) > x_47)) {
    const float4 x_52 = gl_FragCoord;
    const float x_55 = asfloat(x_6[1].x);
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const float x_58 = asfloat(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const float x_60 = asfloat(x_6[2].x);
    icoord = int2(((float2(x_52.x, x_52.y) * x_55) - float2(x_58, x_60)));
    const float x_65 = asfloat(x_6[2].x);
    const float x_67 = asfloat(x_6[3].x);
    const int x_69 = icoord.x;
    const int x_71 = icoord.y;
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_74 = asint(x_9[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    if (((x_69 * x_71) != x_74)) {
      const float x_80 = asfloat(x_6[3].x);
      x_40 = x_80;
    } else {
      const float x_82 = asfloat(x_6[2].x);
      x_40 = x_82;
    }
    const float x_83 = x_40;
    const uint scalar_offset_3 = ((16u * uint(0))) / 4;
    const int x_85 = asint(x_9[scalar_offset_3 / 4][scalar_offset_3 % 4]);
    x_GLF_color = float4(x_65, x_67, x_83, float(x_85));
  } else {
    const float4 x_88 = gl_FragCoord;
    const float x_91 = asfloat(x_6[1].x);
    const uint scalar_offset_4 = ((16u * uint(0))) / 4;
    const float x_94 = asfloat(x_6[scalar_offset_4 / 4][scalar_offset_4 % 4]);
    const float x_96 = asfloat(x_6[2].x);
    icoord_1 = int2(((float2(x_88.x, x_88.y) * x_91) - float2(x_94, x_96)));
    const float x_101 = asfloat(x_6[3].x);
    const float x_103 = asfloat(x_6[3].x);
    const int x_105 = icoord_1.x;
    const float x_108 = asfloat(x_6[3].x);
    x_GLF_color = float4(x_101, x_103, float(x_105), x_108);
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
