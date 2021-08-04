static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_11 : register(b0, space0) {
  uint4 x_11[1];
};

float func_i1_(inout int b) {
  int ndx = 0;
  int i = 0;
  ndx = 0;
  {
    for(; (ndx < 2); ndx = (ndx + 1)) {
      const float x_104 = gl_FragCoord.x;
      if ((x_104 < 0.0f)) {
        i = 0;
        {
          for(; (i < 2); i = (i + 1)) {
            if ((int(cosh(float2(1.0f, 800.0f)).x) <= 1)) {
              discard;
            }
          }
        }
      }
    }
  }
  const int x_125 = b;
  if ((x_125 > 1)) {
    return 3.0f;
  }
  const float x_130 = gl_FragCoord.x;
  if ((x_130 < 0.0f)) {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return 5.0f;
}

void main_1() {
  float f = 0.0f;
  int param = 0;
  int x_1 = 0;
  int param_1 = 0;
  x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
  f = 0.0f;
  while (true) {
    const float x_54 = x_GLF_color.y;
    if ((int(x_54) < 0)) {
      discard;
    } else {
      const float x_61 = asfloat(x_11[0].x);
      param = int(x_61);
      const float x_63 = func_i1_(param);
      f = x_63;
    }
    const float x_65 = x_GLF_color.y;
    if ((int(x_65) > 65)) {
      discard;
    }
    x_1 = 0;
    while (true) {
      const int x_74 = x_1;
      const float x_76 = asfloat(x_11[0].x);
      if ((x_74 < (int(x_76) + 1))) {
      } else {
        break;
      }
      param_1 = (x_1 + 10);
      const float x_83 = func_i1_(param_1);
      f = x_83;
      {
        x_1 = (x_1 + 1);
      }
    }
    {
      const float x_87 = asfloat(x_11[0].x);
      if ((int(x_87) > 1)) {
      } else {
        break;
      }
    }
  }
  if ((f == 3.0f)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
