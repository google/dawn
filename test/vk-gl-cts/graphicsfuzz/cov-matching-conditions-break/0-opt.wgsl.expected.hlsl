static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[3];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const int x_31 = asint(x_6[scalar_offset / 4][scalar_offset % 4]);
  const int x_34 = asint(x_6[1].x);
  const int x_37 = asint(x_6[1].x);
  const uint scalar_offset_1 = ((16u * uint(0))) / 4;
  const int x_40 = asint(x_6[scalar_offset_1 / 4][scalar_offset_1 % 4]);
  x_GLF_color = float4(float(x_31), float(x_34), float(x_37), float(x_40));
  const float x_44 = gl_FragCoord.y;
  if ((x_44 < 0.0f)) {
    const int x_49 = asint(x_6[1].x);
    const float x_50 = float(x_49);
    x_GLF_color = float4(x_50, x_50, x_50, x_50);
  }
  const int x_53 = asint(x_6[1].x);
  i = x_53;
  while (true) {
    const int x_58 = i;
    const int x_60 = asint(x_6[2].x);
    if ((x_58 < x_60)) {
    } else {
      break;
    }
    const float x_64 = gl_FragCoord.x;
    if ((x_64 > 0.0f)) {
      const float x_69 = gl_FragCoord.y;
      if ((x_69 < 0.0f)) {
        const int x_74 = asint(x_6[1].x);
        const float x_75 = float(x_74);
        x_GLF_color = float4(x_75, x_75, x_75, x_75);
        break;
      }
    }
    const float x_78 = gl_FragCoord.x;
    if ((x_78 > 0.0f)) {
      const float x_83 = gl_FragCoord.y;
      if ((x_83 < 0.0f)) {
        const int x_88 = asint(x_6[1].x);
        const float x_89 = float(x_88);
        x_GLF_color = float4(x_89, x_89, x_89, x_89);
      }
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
