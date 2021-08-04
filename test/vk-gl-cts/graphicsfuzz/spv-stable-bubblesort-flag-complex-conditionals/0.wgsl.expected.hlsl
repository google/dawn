static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[1];
};
cbuffer cbuffer_x_13 : register(b0, space0) {
  uint4 x_13[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

bool checkSwap_f1_f1_(inout float a, inout float b) {
  bool x_144 = false;
  const float x_146 = gl_FragCoord.y;
  const float x_148 = asfloat(x_9[0].y);
  if ((x_146 < (x_148 / 2.0f))) {
    const float x_154 = a;
    const float x_155 = b;
    x_144 = (x_154 > x_155);
  } else {
    const float x_157 = a;
    const float x_158 = b;
    x_144 = (x_157 < x_158);
  }
  return x_144;
}

void main_1() {
  int i = 0;
  float data[10] = (float[10])0;
  int i_1 = 0;
  int j = 0;
  bool doSwap = false;
  float param = 0.0f;
  float param_1 = 0.0f;
  float temp = 0.0f;
  i = 0;
  {
    for(; (i < 10); i = (i + 1)) {
      const int x_59 = i;
      const int x_60 = i;
      const float x_64 = asfloat(x_13[0].y);
      data[x_59] = (float((10 - x_60)) * x_64);
    }
  }
  i_1 = 0;
  {
    for(; (i_1 < 9); i_1 = (i_1 + 1)) {
      j = 0;
      {
        for(; (j < 10); j = (j + 1)) {
          if ((j < (i_1 + 1))) {
            continue;
          }
          const int x_90 = j;
          const float x_92 = data[i_1];
          param = x_92;
          const float x_94 = data[x_90];
          param_1 = x_94;
          const bool x_95 = checkSwap_f1_f1_(param, param_1);
          doSwap = x_95;
          if (doSwap) {
            const float x_101 = data[i_1];
            temp = x_101;
            const int x_102 = i_1;
            const float x_105 = data[j];
            data[x_102] = x_105;
            data[j] = temp;
          }
        }
      }
    }
  }
  const float x_115 = gl_FragCoord.x;
  const float x_117 = asfloat(x_9[0].x);
  if ((x_115 < (x_117 / 2.0f))) {
    const float x_124 = data[0];
    const float x_127 = data[5];
    const float x_130 = data[9];
    x_GLF_color = float4((x_124 / 10.0f), (x_127 / 10.0f), (x_130 / 10.0f), 1.0f);
  } else {
    const float x_134 = data[5];
    const float x_137 = data[9];
    const float x_140 = data[0];
    x_GLF_color = float4((x_134 / 10.0f), (x_137 / 10.0f), (x_140 / 10.0f), 1.0f);
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
