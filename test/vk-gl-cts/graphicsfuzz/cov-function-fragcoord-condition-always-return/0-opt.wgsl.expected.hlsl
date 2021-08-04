static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_8 : register(b1, space0) {
  uint4 x_8[4];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[2];
};

float func_f1_(inout float x) {
  while (true) {
    if (true) {
    } else {
      break;
    }
    while (true) {
      const float x_77 = gl_FragCoord.y;
      const float x_79 = asfloat(x_8[2].x);
      if ((x_77 < x_79)) {
        while (true) {
          {
            const float x_88 = gl_FragCoord.x;
            const float x_90 = asfloat(x_8[2].x);
            if ((x_88 < x_90)) {
            } else {
              break;
            }
          }
        }
      }
      const float x_92 = x;
      const float x_94 = asfloat(x_8[3].x);
      if ((x_92 < x_94)) {
        const float x_99 = asfloat(x_8[1].x);
        return x_99;
      }
      {
        const float x_101 = gl_FragCoord.y;
        const float x_103 = asfloat(x_8[2].x);
        if ((x_101 < x_103)) {
        } else {
          break;
        }
      }
    }
  }
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_106 = asfloat(x_8[scalar_offset / 4][scalar_offset % 4]);
  return x_106;
}

void main_1() {
  float param = 0.0f;
  const float x_41 = gl_FragCoord.x;
  param = x_41;
  const float x_42 = func_f1_(param);
  const float x_44 = asfloat(x_8[1].x);
  if ((x_42 == x_44)) {
    const uint scalar_offset_1 = ((16u * uint(0))) / 4;
    const int x_50 = asint(x_11[scalar_offset_1 / 4][scalar_offset_1 % 4]);
    const int x_53 = asint(x_11[1].x);
    const int x_56 = asint(x_11[1].x);
    const uint scalar_offset_2 = ((16u * uint(0))) / 4;
    const int x_59 = asint(x_11[scalar_offset_2 / 4][scalar_offset_2 % 4]);
    x_GLF_color = float4(float(x_50), float(x_53), float(x_56), float(x_59));
  } else {
    const int x_63 = asint(x_11[1].x);
    const float x_64 = float(x_63);
    x_GLF_color = float4(x_64, x_64, x_64, x_64);
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
