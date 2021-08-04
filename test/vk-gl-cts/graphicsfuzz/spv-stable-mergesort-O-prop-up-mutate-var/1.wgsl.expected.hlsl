cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int temp[10] = (int[10])0;
  int data[10] = (int[10])0;
  float x_190 = 0.0f;
  float x_262 = 0.0f;
  int x_63_phi = 0;
  int x_103_phi = 0;
  int x_112_phi = 0;
  float x_263_phi = 0.0f;
  const float x_60 = asfloat(x_8[0].x);
  const int x_61 = int(x_60);
  x_63_phi = x_61;
  while (true) {
    int x_100 = 0;
    int x_98 = 0;
    int x_96 = 0;
    int x_94 = 0;
    int x_92 = 0;
    int x_90 = 0;
    int x_88 = 0;
    int x_86 = 0;
    int x_84 = 0;
    int x_82 = 0;
    int x_64_phi = 0;
    const int x_63 = x_63_phi;
    const int x_68[10] = data;
    const int tint_symbol_4[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    data = tint_symbol_4;
    data = x_68;
    const int x_69 = (x_63 + 1);
    x_64_phi = x_69;
    switch(x_63) {
      case 9: {
        data[x_63] = -5;
        x_100 = (x_63 + 1);
        x_64_phi = x_100;
        break;
      }
      case 8: {
        data[x_63] = -4;
        x_98 = (x_63 + 1);
        x_64_phi = x_98;
        break;
      }
      case 7: {
        data[x_63] = -3;
        x_96 = (x_63 + 1);
        x_64_phi = x_96;
        break;
      }
      case 6: {
        data[x_63] = -2;
        x_94 = (x_63 + 1);
        x_64_phi = x_94;
        break;
      }
      case 5: {
        data[x_63] = -1;
        x_92 = (x_63 + 1);
        x_64_phi = x_92;
        break;
      }
      case 4: {
        data[x_63] = 0;
        x_90 = (x_63 + 1);
        x_64_phi = x_90;
        break;
      }
      case 3: {
        data[x_63] = 1;
        x_88 = (x_63 + 1);
        x_64_phi = x_88;
        break;
      }
      case 2: {
        data[x_63] = 2;
        x_86 = (x_63 + 1);
        x_64_phi = x_86;
        break;
      }
      case 1: {
        data[x_63] = 3;
        x_84 = (x_63 + 1);
        x_64_phi = x_84;
        break;
      }
      case 0: {
        data[x_63] = 4;
        x_82 = (x_63 + 1);
        x_64_phi = x_82;
        break;
      }
      default: {
        break;
      }
    }
    const int x_64 = x_64_phi;
    {
      x_63_phi = x_64;
      if ((x_64 < 10)) {
      } else {
        break;
      }
    }
  }
  x_103_phi = 0;
  while (true) {
    int x_104 = 0;
    const int x_103 = x_103_phi;
    if ((x_103 < 10)) {
    } else {
      break;
    }
    {
      const int x_109 = data[x_103];
      temp[x_103] = x_109;
      x_104 = (x_103 + 1);
      x_103_phi = x_104;
    }
  }
  x_112_phi = 1;
  while (true) {
    int x_113 = 0;
    int x_119_phi = 0;
    const int x_112 = x_112_phi;
    if ((x_112 <= 9)) {
    } else {
      break;
    }
    x_119_phi = 0;
    while (true) {
      int x_131 = 0;
      int x_136 = 0;
      int x_131_phi = 0;
      int x_134_phi = 0;
      int x_136_phi = 0;
      int x_158_phi = 0;
      int x_161_phi = 0;
      int x_171_phi = 0;
      const int x_119 = x_119_phi;
      if ((x_119 < 9)) {
      } else {
        break;
      }
      const int x_125 = (x_119 + x_112);
      const int x_126 = (x_125 - 1);
      const int x_120 = (x_119 + (2 * x_112));
      const int x_129 = min((x_120 - 1), 9);
      x_131_phi = x_119;
      x_134_phi = x_125;
      x_136_phi = x_119;
      while (true) {
        int x_151 = 0;
        int x_154 = 0;
        int x_135_phi = 0;
        int x_137_phi = 0;
        x_131 = x_131_phi;
        const int x_134 = x_134_phi;
        x_136 = x_136_phi;
        bool tint_tmp = (x_136 <= x_126);
        if (tint_tmp) {
          tint_tmp = (x_134 <= x_129);
        }
        if ((tint_tmp)) {
        } else {
          break;
        }
        const int x_143_save = x_136;
        const int x_144 = data[x_143_save];
        const int x_145_save = x_134;
        const int x_146 = data[x_145_save];
        const int x_132 = asint((x_131 + asint(1)));
        if ((x_144 < x_146)) {
          x_151 = asint((x_136 + asint(1)));
          const int x_152 = data[x_143_save];
          temp[x_131] = x_152;
          x_135_phi = x_134;
          x_137_phi = x_151;
        } else {
          x_154 = (x_134 + 1);
          const int x_155 = data[x_145_save];
          temp[x_131] = x_155;
          x_135_phi = x_154;
          x_137_phi = x_136;
        }
        const int x_135 = x_135_phi;
        const int x_137 = x_137_phi;
        {
          x_131_phi = x_132;
          x_134_phi = x_135;
          x_136_phi = x_137;
        }
      }
      x_158_phi = x_131;
      x_161_phi = x_136;
      while (true) {
        int x_159 = 0;
        int x_162 = 0;
        const int x_158 = x_158_phi;
        const int x_161 = x_161_phi;
        bool tint_tmp_1 = (x_161 < 10);
        if (tint_tmp_1) {
          tint_tmp_1 = (x_161 <= x_126);
        }
        if ((tint_tmp_1)) {
        } else {
          break;
        }
        {
          x_159 = (x_158 + 1);
          x_162 = (x_161 + 1);
          const int x_168 = data[x_161];
          temp[x_158] = x_168;
          x_158_phi = x_159;
          x_161_phi = x_162;
        }
      }
      x_171_phi = x_119;
      while (true) {
        int x_172 = 0;
        const int x_171 = x_171_phi;
        if ((x_171 <= x_129)) {
        } else {
          break;
        }
        {
          const int x_177 = temp[x_171];
          data[x_171] = x_177;
          x_172 = (x_171 + 1);
          x_171_phi = x_172;
        }
      }
      {
        x_119_phi = x_120;
      }
    }
    {
      x_113 = (2 * x_112);
      x_112_phi = x_113;
    }
  }
  int x_181 = 0;
  float x_199 = 0.0f;
  float x_261 = 0.0f;
  float x_262_phi = 0.0f;
  const float x_180 = gl_FragCoord.y;
  x_181 = int(x_180);
  if ((x_181 < 30)) {
    const int x_187 = data[0];
    x_190 = (0.5f + (float(x_187) * 0.100000001f));
    x_263_phi = x_190;
  } else {
    float x_208 = 0.0f;
    float x_260 = 0.0f;
    float x_261_phi = 0.0f;
    if ((x_181 < 60)) {
      const int x_196 = data[1];
      x_199 = (0.5f + (float(x_196) * 0.100000001f));
      x_262_phi = x_199;
    } else {
      float x_217 = 0.0f;
      float x_259 = 0.0f;
      float x_260_phi = 0.0f;
      if ((x_181 < 90)) {
        const int x_205 = data[2];
        x_208 = (0.5f + (float(x_205) * 0.100000001f));
        x_261_phi = x_208;
      } else {
        if ((x_181 < 120)) {
          const int x_214 = data[3];
          x_217 = (0.5f + (float(x_214) * 0.100000001f));
          x_260_phi = x_217;
        } else {
          float x_230 = 0.0f;
          float x_258 = 0.0f;
          float x_259_phi = 0.0f;
          if ((x_181 < 150)) {
            discard;
          } else {
            float x_239 = 0.0f;
            float x_257 = 0.0f;
            float x_258_phi = 0.0f;
            if ((x_181 < 180)) {
              const int x_227 = data[5];
              x_230 = (0.5f + (float(x_227) * 0.100000001f));
              x_259_phi = x_230;
            } else {
              float x_248 = 0.0f;
              float x_256 = 0.0f;
              float x_257_phi = 0.0f;
              if ((x_181 < 210)) {
                const int x_236 = data[6];
                x_239 = (0.5f + (float(x_236) * 0.100000001f));
                x_258_phi = x_239;
              } else {
                if ((x_181 < 240)) {
                  const int x_245 = data[7];
                  x_248 = (0.5f + (float(x_245) * 0.100000001f));
                  x_257_phi = x_248;
                } else {
                  if ((x_181 < 270)) {
                  } else {
                    discard;
                  }
                  const int x_253 = data[8];
                  x_256 = (0.5f + (float(x_253) * 0.100000001f));
                  x_257_phi = x_256;
                }
                x_257 = x_257_phi;
                x_258_phi = x_257;
              }
              x_258 = x_258_phi;
              x_259_phi = x_258;
            }
            x_259 = x_259_phi;
          }
          x_260_phi = x_259;
        }
        x_260 = x_260_phi;
        x_261_phi = x_260;
      }
      x_261 = x_261_phi;
      x_262_phi = x_261;
    }
    x_262 = x_262_phi;
    x_263_phi = x_262;
  }
  const float x_263 = x_263_phi;
  x_GLF_color = float4(x_263, x_263, x_263, 1.0f);
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
