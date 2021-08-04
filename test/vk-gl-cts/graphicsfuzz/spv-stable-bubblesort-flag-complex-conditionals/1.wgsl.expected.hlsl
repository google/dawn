static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_9 : register(b1, space0) {
  uint4 x_9[1];
};
cbuffer cbuffer_x_13 : register(b0, space0) {
  uint4 x_13[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

bool checkSwap_f1_f1_(inout float a, inout float b) {
  bool x_147 = false;
  float x_158 = 0.0f;
  float x_159 = 0.0f;
  float x_179 = 0.0f;
  float x_178 = 0.0f;
  float x_185 = 0.0f;
  float x_184 = 0.0f;
  float x_160_phi = 0.0f;
  float x_180_phi = 0.0f;
  float x_186_phi = 0.0f;
  const float x_149 = gl_FragCoord.y;
  const float x_151 = asfloat(x_9[0].y);
  const bool x_153 = (x_149 < (x_151 / 2.0f));
  if (x_153) {
    x_158 = a;
    x_160_phi = x_158;
  } else {
    x_159 = 0.0f;
    x_160_phi = x_159;
  }
  float x_166 = 0.0f;
  float x_167 = 0.0f;
  float x_168_phi = 0.0f;
  const float x_160 = x_160_phi;
  bool guard155 = true;
  if (false) {
  } else {
    if (guard155) {
      if (x_153) {
        x_166 = b;
        x_168_phi = x_166;
      } else {
        x_167 = 0.0f;
        x_168_phi = x_167;
      }
      const bool x_169 = (x_160 > x_168_phi);
      if (x_153) {
        x_147 = x_169;
      }
      if (true) {
      } else {
        guard155 = false;
      }
      if (guard155) {
        guard155 = false;
      }
    }
  }
  if (x_153) {
    x_179 = 0.0f;
    x_180_phi = x_179;
  } else {
    x_178 = a;
    x_180_phi = x_178;
  }
  const float x_180 = x_180_phi;
  if (x_153) {
    x_185 = 0.0f;
    x_186_phi = x_185;
  } else {
    x_184 = b;
    x_186_phi = x_184;
  }
  const float x_186 = x_186_phi;
  if (x_153) {
  } else {
    x_147 = (x_180 < x_186);
  }
  return x_147;
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
      const int x_62 = i;
      const int x_63 = i;
      const float x_67 = asfloat(x_13[0].y);
      data[x_62] = (float((10 - x_63)) * x_67);
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
          const int x_93 = j;
          const float x_95 = data[i_1];
          param = x_95;
          const float x_97 = data[x_93];
          param_1 = x_97;
          const bool x_98 = checkSwap_f1_f1_(param, param_1);
          doSwap = x_98;
          if (doSwap) {
            const float x_104 = data[i_1];
            temp = x_104;
            const int x_105 = i_1;
            const float x_108 = data[j];
            data[x_105] = x_108;
            data[j] = temp;
          }
        }
      }
    }
  }
  const float x_118 = gl_FragCoord.x;
  const float x_120 = asfloat(x_9[0].x);
  if ((x_118 < (x_120 / 2.0f))) {
    const float x_127 = data[0];
    const float x_130 = data[5];
    const float x_133 = data[9];
    x_GLF_color = float4((x_127 / 10.0f), (x_130 / 10.0f), (x_133 / 10.0f), 1.0f);
  } else {
    const float x_137 = data[5];
    const float x_140 = data[9];
    const float x_143 = data[0];
    x_GLF_color = float4((x_137 / 10.0f), (x_140 / 10.0f), (x_143 / 10.0f), 1.0f);
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
