cbuffer cbuffer_x_9 : register(b0, space0) {
  uint4 x_9[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float x_43 = 0.0f;
  float x_44 = 0.0f;
  float x_45 = 0.0f;
  int x_46 = 0;
  int zero = 0;
  float2 param = float2(0.0f, 0.0f);
  float2 temp = float2(0.0f, 0.0f);
  const float4 x_47 = gl_FragCoord;
  param = float2(x_47.x, x_47.y);
  while (true) {
    const float x_54 = param.y;
    if ((x_54 < 50.0f)) {
      const float x_60 = asfloat(x_9[0].y);
      x_44 = x_60;
    } else {
      x_44 = 0.0f;
    }
    const float x_61 = x_44;
    x_43 = x_61;
    const float x_63 = gl_FragCoord.y;
    const float x_65 = ((x_63 < 50.0f) ? 1.0f : 0.0f);
    x_45 = x_65;
    if (((x_61 - x_65) < 1.0f)) {
      x_46 = 0;
      break;
    }
    x_46 = 1;
    break;
    {
      if (false) {
      } else {
        break;
      }
    }
  }
  const int x_70 = x_46;
  zero = x_70;
  if ((x_70 == 1)) {
    return;
  }
  x_GLF_color = float4(0.0f, 1.0f, 1.0f, 1.0f);
  const float x_75 = gl_FragCoord.x;
  const float x_77 = asfloat(x_9[0].x);
  if ((x_75 >= x_77)) {
    const float x_82 = gl_FragCoord.y;
    if ((x_82 >= 0.0f)) {
      const float x_87 = asfloat(x_9[0].y);
      x_GLF_color.x = x_87;
    }
  }
  const float x_90 = gl_FragCoord.y;
  if ((x_90 >= 0.0f)) {
    const float x_95 = asfloat(x_9[0].x);
    x_GLF_color.y = x_95;
  }
  const float4 x_97 = gl_FragCoord;
  const float2 x_98 = float2(x_97.x, x_97.y);
  const float2 x_101 = float2(x_98.x, x_98.y);
  temp = x_101;
  if ((x_101.y >= 0.0f)) {
    const float x_107 = asfloat(x_9[0].x);
    x_GLF_color.z = x_107;
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

int alwaysZero_vf2_(inout float2 coord) {
  float a = 0.0f;
  float x_110 = 0.0f;
  float b = 0.0f;
  const float x_112 = coord.y;
  if ((x_112 < 50.0f)) {
    const float x_118 = asfloat(x_9[0].y);
    x_110 = x_118;
  } else {
    x_110 = 0.0f;
  }
  const float x_119 = x_110;
  a = x_119;
  const float x_121 = gl_FragCoord.y;
  const float x_123 = ((x_121 < 50.0f) ? 1.0f : 0.0f);
  b = x_123;
  if (((x_119 - x_123) < 1.0f)) {
    return 0;
  }
  return 1;
}
