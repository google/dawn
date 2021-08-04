static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float func_() {
  int x = 0;
  const float x_99 = gl_FragCoord.x;
  if ((x_99 < 1.0f)) {
    return 5.0f;
  }
  const float x_104 = asfloat(x_7[0].x);
  const float x_106 = asfloat(x_7[0].y);
  if ((x_104 > x_106)) {
    return 1.0f;
  }
  const float x_111 = asfloat(x_7[0].x);
  x = int(x_111);
  const float x_114 = asfloat(x_7[0].x);
  x = (x + (int(clamp(x_114, 0.0f, 1.0f)) * 3));
  return (5.0f + float(x));
}

void main_1() {
  int i = 0;
  int j = 0;
  float2 data[17] = (float2[17])0;
  i = 0;
  while (true) {
    const int x_48 = i;
    const float x_50 = asfloat(x_7[0].x);
    if ((x_48 < (4 + int(x_50)))) {
    } else {
      break;
    }
    const float x_56 = gl_FragCoord.x;
    if ((x_56 >= 0.0f)) {
      j = 0;
      while (true) {
        bool x_81 = false;
        bool x_82_phi = false;
        if ((j < 4)) {
        } else {
          break;
        }
        const int x_67 = j;
        const int x_69 = i;
        const float x_71 = func_();
        data[((4 * x_67) + x_69)].x = x_71;
        const float x_74 = data[0].x;
        const bool x_75 = (x_74 == 5.0f);
        x_82_phi = x_75;
        if (!(x_75)) {
          const float x_80 = data[15].x;
          x_81 = (x_80 == 5.0f);
          x_82_phi = x_81;
        }
        if (x_82_phi) {
          x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
        } else {
          x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
        }
        const float x_87 = asfloat(x_7[0].x);
        const float x_89 = asfloat(x_7[0].y);
        if ((x_87 > x_89)) {
          return;
        }
        {
          j = (j + 1);
        }
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
