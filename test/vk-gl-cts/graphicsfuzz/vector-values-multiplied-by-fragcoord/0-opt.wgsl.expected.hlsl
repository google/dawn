static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_10 : register(b0, space0) {
  uint4 x_10[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float func_() {
  bool alwaysFalse = false;
  float4 value = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float2 a = float2(0.0f, 0.0f);
  int i = 0;
  bool x_121 = false;
  bool x_122_phi = false;
  const float x_71 = gl_FragCoord.x;
  alwaysFalse = (x_71 < -1.0f);
  if (alwaysFalse) {
    const float2 x_76 = a;
    const float4 x_77 = value;
    value = float4(x_76.x, x_76.y, x_77.z, x_77.w);
  }
  if (!(alwaysFalse)) {
    const float2 x_84 = asfloat(x_10[0].xy);
    const float4 x_85 = value;
    value = float4(x_84.x, x_84.y, x_85.z, x_85.w);
  }
  const float4 x_87 = gl_FragCoord;
  const float4 x_89 = value;
  const float4 x_93 = value;
  const float2 x_95 = (((float2(x_87.x, x_87.y) * float2(x_89.x, x_89.y)) * float2(2.0f, 2.0f)) + float2(x_93.x, x_93.y));
  const float4 x_96 = value;
  value = float4(x_96.x, x_96.y, x_95.x, x_95.y);
  i = 0;
  while (true) {
    const int x_102 = i;
    const float x_104 = asfloat(x_10[0].y);
    if ((x_102 < (int(x_104) + 1))) {
    } else {
      break;
    }
    value.x = float(i);
    {
      i = (i + 1);
    }
  }
  const float x_115 = value.x;
  const bool x_116 = (x_115 == 1.0f);
  x_122_phi = x_116;
  if (x_116) {
    const float x_120 = value.y;
    x_121 = (x_120 == 1.0f);
    x_122_phi = x_121;
  }
  if (x_122_phi) {
    return 1.0f;
  } else {
    return 0.0f;
  }
  return 0.0f;
}

void main_1() {
  int count = 0;
  int i_1 = 0;
  count = 0;
  i_1 = 0;
  while (true) {
    const int x_51 = i_1;
    const float x_53 = asfloat(x_10[0].y);
    if ((x_51 < (int(x_53) + 1))) {
    } else {
      break;
    }
    const float x_58 = func_();
    count = (count + int(x_58));
    {
      i_1 = (i_1 + 1);
    }
  }
  if ((count == 2)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
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
