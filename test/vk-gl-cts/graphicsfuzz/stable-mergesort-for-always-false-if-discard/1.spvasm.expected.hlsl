cbuffer cbuffer_x_8 : register(b0, space0) {
  uint4 x_8[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int temp[10] = (int[10])0;
  int data[10] = (int[10])0;
  float x_180 = 0.0f;
  float x_279 = 0.0f;
  int x_65_phi = 0;
  int x_93_phi = 0;
  int x_102_phi = 0;
  float x_280_phi = 0.0f;
  const float x_62 = asfloat(x_8[0].x);
  const int x_63 = int(x_62);
  x_65_phi = x_63;
  while (true) {
    const int x_65 = x_65_phi;
    switch(x_65) {
      case 9: {
        data[x_65] = -5;
        break;
      }
      case 8: {
        data[x_65] = -4;
        break;
      }
      case 7: {
        data[x_65] = -3;
        break;
      }
      case 6: {
        data[x_65] = -2;
        break;
      }
      case 5: {
        data[x_65] = -1;
        break;
      }
      case 4: {
        data[x_65] = 0;
        break;
      }
      case 3: {
        data[x_65] = 1;
        break;
      }
      case 2: {
        data[x_65] = 2;
        break;
      }
      case 1: {
        data[x_65] = 3;
        break;
      }
      case 0: {
        data[x_65] = 4;
        break;
      }
      default: {
        break;
      }
    }
    const int x_66 = (x_65 + 1);
    {
      x_65_phi = x_66;
      if ((x_66 < 10)) {
      } else {
        break;
      }
    }
  }
  x_93_phi = 0;
  while (true) {
    int x_94 = 0;
    const int x_93 = x_93_phi;
    if ((x_93 < 10)) {
    } else {
      break;
    }
    {
      const int x_99 = data[x_93];
      temp[x_93] = x_99;
      x_94 = (x_93 + 1);
      x_93_phi = x_94;
    }
  }
  x_102_phi = 1;
  while (true) {
    int x_103 = 0;
    int x_109_phi = 0;
    const int x_102 = x_102_phi;
    if ((x_102 <= 9)) {
    } else {
      break;
    }
    x_109_phi = 0;
    while (true) {
      int x_121 = 0;
      int x_126 = 0;
      int x_121_phi = 0;
      int x_124_phi = 0;
      int x_126_phi = 0;
      int x_148_phi = 0;
      int x_151_phi = 0;
      int x_161_phi = 0;
      const int x_109 = x_109_phi;
      if ((x_109 < 9)) {
      } else {
        break;
      }
      const int x_115 = (x_109 + x_102);
      const int x_116 = (x_115 - 1);
      const int x_110 = (x_109 + (2 * x_102));
      const int x_119 = min((x_110 - 1), 9);
      x_121_phi = x_109;
      x_124_phi = x_115;
      x_126_phi = x_109;
      while (true) {
        int x_141 = 0;
        int x_144 = 0;
        int x_125_phi = 0;
        int x_127_phi = 0;
        x_121 = x_121_phi;
        const int x_124 = x_124_phi;
        x_126 = x_126_phi;
        if (((x_126 <= x_116) & (x_124 <= x_119))) {
        } else {
          break;
        }
        const int x_133_save = x_126;
        const int x_134 = data[x_133_save];
        const int x_135_save = x_124;
        const int x_136 = data[x_135_save];
        const int x_122 = asint((x_121 + asint(1)));
        if ((x_134 < x_136)) {
          x_141 = asint((x_126 + asint(1)));
          const int x_142 = data[x_133_save];
          temp[x_121] = x_142;
          x_125_phi = x_124;
          x_127_phi = x_141;
        } else {
          x_144 = (x_124 + 1);
          const int x_145 = data[x_135_save];
          temp[x_121] = x_145;
          x_125_phi = x_144;
          x_127_phi = x_126;
        }
        const int x_125 = x_125_phi;
        const int x_127 = x_127_phi;
        {
          x_121_phi = x_122;
          x_124_phi = x_125;
          x_126_phi = x_127;
        }
      }
      x_148_phi = x_121;
      x_151_phi = x_126;
      while (true) {
        int x_149 = 0;
        int x_152 = 0;
        const int x_148 = x_148_phi;
        const int x_151 = x_151_phi;
        if (((x_151 < 10) & (x_151 <= x_116))) {
        } else {
          break;
        }
        {
          x_149 = (x_148 + 1);
          x_152 = (x_151 + 1);
          const int x_158 = data[x_151];
          temp[x_148] = x_158;
          x_148_phi = x_149;
          x_151_phi = x_152;
        }
      }
      x_161_phi = x_109;
      while (true) {
        int x_162 = 0;
        const int x_161 = x_161_phi;
        if ((x_161 <= x_119)) {
        } else {
          break;
        }
        {
          const int x_167 = temp[x_161];
          data[x_161] = x_167;
          x_162 = (x_161 + 1);
          x_161_phi = x_162;
        }
      }
      {
        x_109_phi = x_110;
      }
    }
    {
      x_103 = (2 * x_102);
      x_102_phi = x_103;
    }
  }
  int x_171 = 0;
  float x_189 = 0.0f;
  float x_278 = 0.0f;
  float x_279_phi = 0.0f;
  const float x_170 = gl_FragCoord.y;
  x_171 = int(x_170);
  if ((x_171 < 30)) {
    const int x_177 = data[0];
    x_180 = (0.5f + (float(x_177) * 0.100000001f));
    x_280_phi = x_180;
  } else {
    float x_198 = 0.0f;
    float x_277 = 0.0f;
    float x_278_phi = 0.0f;
    if ((x_171 < 60)) {
      const int x_186 = data[1];
      x_189 = (0.5f + (float(x_186) * 0.100000001f));
      x_279_phi = x_189;
    } else {
      float x_207 = 0.0f;
      float x_249 = 0.0f;
      float x_277_phi = 0.0f;
      if ((x_171 < 90)) {
        const int x_195 = data[2];
        x_198 = (0.5f + (float(x_195) * 0.100000001f));
        x_278_phi = x_198;
      } else {
        if ((x_171 < 120)) {
          const int x_204 = data[3];
          x_207 = (0.5f + (float(x_204) * 0.100000001f));
          x_277_phi = x_207;
        } else {
          float x_220 = 0.0f;
          float x_248 = 0.0f;
          float x_249_phi = 0.0f;
          float2 x_256_phi = float2(0.0f, 0.0f);
          int x_259_phi = 0;
          if ((x_171 < 150)) {
            discard;
          } else {
            float x_229 = 0.0f;
            float x_247 = 0.0f;
            float x_248_phi = 0.0f;
            if ((x_171 < 180)) {
              const int x_217 = data[5];
              x_220 = (0.5f + (float(x_217) * 0.100000001f));
              x_249_phi = x_220;
            } else {
              float x_238 = 0.0f;
              float x_246 = 0.0f;
              float x_247_phi = 0.0f;
              if ((x_171 < 210)) {
                const int x_226 = data[6];
                x_229 = (0.5f + (float(x_226) * 0.100000001f));
                x_248_phi = x_229;
              } else {
                if ((x_171 < 240)) {
                  const int x_235 = data[7];
                  x_238 = (0.5f + (float(x_235) * 0.100000001f));
                  x_247_phi = x_238;
                } else {
                  if ((x_171 < 270)) {
                  } else {
                    discard;
                  }
                  const int x_243 = data[8];
                  x_246 = (0.5f + (float(x_243) * 0.100000001f));
                  x_247_phi = x_246;
                }
                x_247 = x_247_phi;
                x_248_phi = x_247;
              }
              x_248 = x_248_phi;
              x_249_phi = x_248;
            }
            x_249 = x_249_phi;
            const float x_251 = asfloat(x_8[0].y);
            const bool x_252 = (x_62 > x_251);
            if (x_252) {
              x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
            }
            x_256_phi = float2(1.0f, 1.0f);
            x_259_phi = 0;
            while (true) {
              float2 x_272 = float2(0.0f, 0.0f);
              int x_260 = 0;
              float2 x_273_phi = float2(0.0f, 0.0f);
              const float2 x_256 = x_256_phi;
              const int x_259 = x_259_phi;
              if ((x_259 <= 32)) {
              } else {
                break;
              }
              x_273_phi = x_256;
              if ((x_256.x < 0.0f)) {
                if (x_252) {
                  discard;
                }
                x_272 = x_256;
                x_272.y = (x_256.y + 1.0f);
                x_273_phi = x_272;
              }
              const float2 x_273 = x_273_phi;
              float2 x_257_1 = x_273;
              x_257_1.x = (x_273.x + x_273.y);
              const float2 x_257 = x_257_1;
              {
                x_260 = (x_259 + 1);
                x_256_phi = x_257;
                x_259_phi = x_260;
              }
            }
          }
          x_277_phi = x_249;
        }
        x_277 = x_277_phi;
        x_278_phi = x_277;
      }
      x_278 = x_278_phi;
      x_279_phi = x_278;
    }
    x_279 = x_279_phi;
    x_280_phi = x_279;
  }
  const float x_280 = x_280_phi;
  x_GLF_color = float4(x_280, x_280, x_280, 1.0f);
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
